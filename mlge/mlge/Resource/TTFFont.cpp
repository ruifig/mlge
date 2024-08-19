#include "mlge/Render/Renderer.h"
#include "mlge/Resource/TTFFont.h"

#include "utf8.h"
#include "utf8/unchecked.h"

namespace mlge
{

//////////////////////////////////////////////////////////////////////////
//			MTTFFontDefinition
//////////////////////////////////////////////////////////////////////////

ObjectPtr<MResource> MTTFFontDefinition::create() const
{
	ObjectPtr<MTTFFont> resource = createObject<MTTFFont>(*this);
	if (resource)
	{
		const MTTFFont::Instance* instance = resource->loadASCIIGlyphs(m_defaultPtSize);
		if (instance)
		{
			return resource;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

void MTTFFontDefinition::to_json(nlohmann::json& j) const
{
	Super::to_json(j);
	j["renderMethod"] = m_renderMethod;

	if (m_defaultPtSize != defaultPtSize)
	{
		j["defaultPtSize"] = m_defaultPtSize;
	}

	j["bold"] = m_bold;;
}

void MTTFFontDefinition::from_json(const nlohmann::json& j)
{
	Super::from_json(j);
	j.at("renderMethod").get_to(m_renderMethod);
	if (j.count("defaultPtSize") != 0)
	{
		j.at("defaultPtSize").get_to(m_defaultPtSize);
	}

	if (j.count("bold") != 0)
	{
		j.at("bold").get_to(m_bold);
	}
}


//////////////////////////////////////////////////////////////////////////
//		MTTFFontDefinition Editor
//////////////////////////////////////////////////////////////////////////
#if MLGE_EDITOR
namespace editor
{

} // namespace editor

std::unique_ptr<editor::BaseResourceWindow> MTTFFontDefinition::createEditWindow()
{
	return nullptr;
}
#endif

//////////////////////////////////////////////////////////////////////////
//			MTTFFont
//////////////////////////////////////////////////////////////////////////

MTTFFont::~MTTFFont()
{
}

namespace
{

	// Set this to 1 to write the atlas PNG files to the game's exe folder. It helps figuring out bugs in the glyph creation
	#define DEBUG_GLYPH_CREATION 0

	struct AtlasSurface
	{
		// Extra pixels of padding around each glyph to avoid filtering artifacts
		inline static constexpr uint16_t padding = 1;
		Point cursor = { padding, padding };

		// As we add glyphs to the surface per row, we remember the biggest glyph, so when we skip to the next row, we skip
		// by this many pixels
		int currentRowHeight = 0;
		int maxX = 0;

		/** Where in the texture each glyph is position */
		std::unordered_map<uint32_t, Rect> glyphRects;

		Rect area = {};

		/**
		 * Returns the total size used to hold all the gpyhs
		 */
		Size getUsedSize() const
		{
			return {maxX + 1, cursor.y + currentRowHeight};
		}

		/**
		 * Trims the surface to the minimum size required to hold the glyphs.
		 * This should be called at the end, before creating a GPU Texture
		 */
		void trim()
		{
			Size usedSize = getUsedSize();
			SDLUniquePtr<SDL_Surface> newSurface = createSurface(usedSize.w, usedSize.h);
			SDL_Rect copyRect;
			copyRect.x = 0;
			copyRect.y = 0;
			copyRect.w = usedSize.w;
			copyRect.h = usedSize.h;
			
			CZ_VERIFY(SDL_BlitSurface(surface.get(), &copyRect, newSurface.get(), &copyRect) == 0);

			std::swap(newSurface, surface);
		}

		/**
		 * Tries to pack a glyph into the surface.
		 * Returns true if succeeded, false if the surface ran out of space.
		 */
		bool packGlyph(uint32_t codepoint, Size size, Rect& out)
		{
			Rect tmp(cursor, size.w + padding, size.h + padding); 

			// If the glyph doesn't fit in the current row, move to the next row
			if (!area.contains(tmp))
			{
				tmp = Rect(padding, cursor.y + currentRowHeight, size.w + padding, size.h + padding); 
				if (!area.contains(tmp))
				{
					// Once a single glyph fails to fit, we consider the surface ran out of space.
					return false;
				}
			}

			cursor = {tmp.right() + 1, tmp.top()};
			if (tmp.h > currentRowHeight)
			{
				currentRowHeight = tmp.h;
			}

			if (tmp.right() > maxX)
			{
				maxX = tmp.right();
			}

			// The size of the returned rectangle should not include the padding
			out = Rect(tmp.x, tmp.y, tmp.w - padding, tmp.h - padding);
			glyphRects[codepoint] = out;

			if constexpr(DEBUG_GLYPH_CREATION)
			{
				static int counter=0;
				uint32_t colour = (counter++)%2 ? 0xFF7F00FF : 0xFFFF007F;
				SDL_FillRect(surface.get(), &out, colour);
			}

			return true;
		}

		/**
		 * Creates the surface
		 */
		bool create(int width, int height)
		{
			surface = createSurface(width, height);
			if (!surface.get())
			{
				return false;
			}

			area = Rect(0, 0, surface->w, surface->h);
			return true;
		}

		/**
		 * Saves the surface to a png file.
		 */
		void saveToFile(const fs::path& path)
		{
			IMG_SavePNG(surface.get(), path.string().c_str());
		}

		/**
		 * Creates an Texture from the surface.
		 */
		SDLUniquePtr<SDL_Texture> createTexture()
		{
			// Create a new Surface just with the minimum size required, so we don't want VRAM
			trim();

			if constexpr(DEBUG_GLYPH_CREATION)
			{
				static int counter = 0;
				saveToFile(getProcessPath() / std::format("glyps_{}.png", counter));
				counter++;
			}

			SDLUniquePtr<SDL_Texture> texture(SDL_CreateTextureFromSurface(Renderer::get().getSDLRenderer(), surface.get()));
			CZ_CHECK(texture);
			return texture;
		};

		SDLUniquePtr<SDL_Surface> surface;

	  private:
		SDLUniquePtr<SDL_Surface> createSurface(int width, int height)
		{
			return SDLUniquePtr<SDL_Surface>(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000));
			//return SDLUniquePtr<SDL_Surface>(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8 , 0x00000000, 0x00000000, 0x00000000, 0xFF));
		}

	};
}

bool MTTFFont::Instance::loadGlyphs(const MTTFFontDefinition& def, std::string_view str)
{
	// First check if indeed we need to load any glyphs
	{
		utf8::unchecked::iterator<const char*> it(str.data());
		bool needsLoading = false;
		while(*it)
		{
			uint32_t codePoint = *it;
			++it;

			if (glyphs.find(codePoint) == glyphs.end())
			{
				needsLoading = true;
				break;
			}
		}

		if (!needsLoading)
		{
			return true;
		}
	}

	auto fullpath = def.getRoot().path / def.file;
	SDLUniquePtr<TTF_Font> font(TTF_OpenFont(fullpath.string().c_str(), ptsize));

	if (!font)
	{
		CZ_LOG(
			Error, "Failed to load point size {} for font {}, from file{}. ec={}", ptsize, def.name, narrow(def.file.native()),
			SDL_GetError());
		return false;
	}

	TTF_SetFontHinting(font.get(), TTF_HINTING_LIGHT_SUBPIXEL);
	int style = 0;
	if (def.isBold())
	{
		style |= TTF_STYLE_BOLD;
	}
	TTF_SetFontStyle(font.get(), style);

	height = TTF_FontHeight(font.get());
	ascent = TTF_FontAscent(font.get());
	descent = TTF_FontDescent(font.get());
	lineSkip = TTF_FontLineSkip(font.get());

	// Some bug for certain fonts can result in an incorrect height.
	if (height < (ascent - descent))
	{
		height = ascent - descent;
	}

	AtlasSurface atlas;

	// Create a surface big enough to hold a bunch of glyphs. 
	atlas.create(height*12, height*12);

	auto createTextureFromAtlas = [this, &atlas]()
	{
		SDLUniquePtr<SDL_Texture> texture =  atlas.createTexture();

		for(const std::pair<uint32_t, Rect>& entry : atlas.glyphRects)
		{
			Glyph& glyph = glyphs[entry.first];
			glyph.rect = entry.second;
			glyph.texture = texture.get();
		}

		textures.push_back(std::move(texture));
	};

	utf8::unchecked::iterator<const char*> charIt(str.data());
	while(*charIt)
	{
		const uint32_t codepoint = *charIt;
		++charIt;

		SDL_Color white = {255, 255, 255, 255};
		SDLUniquePtr<SDL_Surface> glyphSurface;

		// For tab, we hardcode a glyph that is X spaces wide
		if (codepoint == 9)
		{
			std::string buf;
			buf.append(static_cast<size_t>(tabWidth), ' ');

			switch(def.getRenderMethod())
			{
				case MTTFFontDefinition::RenderMethod::Solid:
					glyphSurface.reset(TTF_RenderUTF8_Solid(font.get(), buf.c_str(), white));
					break;
				case MTTFFontDefinition::RenderMethod::Blended:
					glyphSurface.reset(TTF_RenderUTF8_Blended(font.get(), buf.c_str(), white));
					break;
			}
		}
		else
		{
			switch(def.getRenderMethod())
			{
				case MTTFFontDefinition::RenderMethod::Solid:
					glyphSurface.reset(TTF_RenderGlyph32_Solid(font.get(), codepoint, white));
					break;
				case MTTFFontDefinition::RenderMethod::Blended:
					glyphSurface.reset(TTF_RenderGlyph32_Blended(font.get(), codepoint, white));
					break;
			}
		}

		if (!glyphSurface)
		{
			CZ_LOG(Error, "Failed to load codepoint {} for font {}", codepoint, def.name);
			continue;
		}

		auto doGlyph = [&atlas, &font, this](uint32_t codepoint, SDL_Surface* glyphSurface) -> bool
		{
			Rect glyphRect;
			if (atlas.packGlyph(codepoint, {glyphSurface->w, glyphSurface->h}, glyphRect))
			{
				// Blit the glyph individual surface to the atlas surface
				SDL_SetSurfaceBlendMode(glyphSurface, (DEBUG_GLYPH_CREATION) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
				SDL_Rect srcRect = {0, 0, glyphSurface->w, glyphSurface->h};
				CZ_VERIFY(SDL_BlitSurface(glyphSurface, &srcRect, atlas.surface.get(), &glyphRect) == 0);

				{
					Glyph& glyph = glyphs[codepoint];
					TTF_GlyphMetrics32(font.get(), codepoint, &glyph.minx, &glyph.maxx, &glyph.miny, &glyph.maxy, &glyph.advance);

					if (codepoint == 9)
					{
						glyph.advance *= tabWidth;
					}

					CZ_LOG(
						VeryVerbose, "ch '{}' : minx={}, maxx={}, miny={}, maxy={}, advance={}", char(codepoint),
						glyph.minx, glyph.maxx, glyph.miny, glyph.maxy, glyph.advance);

				}

				return true;
			}
			else
			{
				return false;
			}
		};

		if (!doGlyph(codepoint, glyphSurface.get()))
		{
			CZ_CHECK(atlas.glyphRects.size());
			createTextureFromAtlas();

			// Create new atlas
			atlas = AtlasSurface();
			atlas.create(height*12, height*12);

			CZ_VERIFY(doGlyph(codepoint,glyphSurface.get()));
		}

	}

	if (atlas.glyphRects.size())
	{
		createTextureFromAtlas();
	}

	if (TTF_FontFaceIsFixedWidth(font.get()) == 0)
	{
		monospaced = {};
	}
	else
	{
		monospaced = glyphs.begin()->second.advance;
	}

	return true;
}

const MTTFFont::Instance* MTTFFont::loadGlyphs(std::string_view str, int ptsize)
{
	for(auto&& instance : m_instances)
	{
		if (instance->ptsize == ptsize)
		{
			instance->loadGlyphs(getDefinition(), str);
			return instance.get();
		}
	}

	auto instance = std::make_unique<MTTFFont::Instance>(ptsize);
	if (!instance->loadGlyphs(getDefinition(), str))
	{
		return nullptr;
	}

	MTTFFont::Instance* ret = instance.get();
	m_instances.push_back(std::move(instance));
	return ret;
}

const MTTFFont::Instance* MTTFFont::loadASCIIGlyphs(int ptsize)
{
	std::string str;
	str.reserve(1 + (126-32+1));
	for(char ch=32; ch<127; ch++)
	{
		str.push_back(ch);
	}

	// tab need to go a the end, so "space" is already setup, since the tab glyph needs to know how wide the space glyph is
	str.push_back(9); // Tab

	return loadGlyphs(str, ptsize);
}

const MTTFFont::Instance* MTTFFont::getPtSize(int ptsize)
{
	for(auto&& instance : m_instances)
	{
		if (instance->ptsize == ptsize)
		{
			return instance.get();
		}
	}

	return loadASCIIGlyphs(ptsize);
}

} // namespace mlge


