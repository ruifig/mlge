#pragma once

#include "mlge/Resource/Resource.h"
#include "mlge/Math.h"

namespace mlge
{

MLGE_OBJECT_START(MTTFFontDefinition, MResourceDefinition, "TTF Font")
class MTTFFontDefinition : public MResourceDefinition
{
	MLGE_OBJECT_INTERNALS(MTTFFontDefinition, MResourceDefinition)

  public:

	enum class RenderMethod
	{
		Solid,
		Blended
	};

	RenderMethod getRenderMethod() const
	{
		return m_renderMethod;
	}

	bool isBold() const
	{
		return m_bold;
	}

  protected:

	virtual void to_json(nlohmann::json& j) const override;
	virtual void from_json(const nlohmann::json& j) override;

	virtual ObjectPtr<MResource> create() const override;

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() override;
	#endif

	static constexpr int defaultPtSize = 14;
	int m_defaultPtSize = defaultPtSize;
	bool m_bold = false;

	RenderMethod m_renderMethod = RenderMethod::Blended;
};
MLGE_OBJECT_END(MTTFFontDefinition)


MLGE_OBJECT_START(MTTFFont, MResource, "TTF Font")
class MTTFFont : public MResource
{
	MLGE_OBJECT_INTERNALS(MTTFFont, MResource);
  public:

	/**
	 * A single glyph's data
	 */
	struct Glyph
	{

		/**
		 * Metrics
		 * See https://freetype.sourceforge.net/freetype2/docs/tutorial/step2.html
		 */
		int minx = 0;
		int maxx = 0;
		int miny = 0;
		int maxy = 0;
		int advance = 0;
		
		/** Portion of the texture where the glyph is located */
		Rect rect;

		/** Texture where the glyph is located */
		SDL_Texture* texture;
	};

	/**
	 * Puts together all the information for a specific point size of the font
	 */
	struct Instance
	{
		explicit Instance(int ptsize)
			: ptsize(ptsize)
		{
		}

		/** If set, then the font is monospaced and the value is the glyphs "advance" */
		std::optional<int> monospaced = {};

		int ptsize;
		int tabWidth = 4;

		int height;
		int ascent;
		int descent;
		int lineSkip;

		/**
		 * All the loaded glyphs. The key is the u32 codepoint
		 */
		std::unordered_map<uint32_t, Glyph> glyphs;

		const Glyph* getGlyph(uint32_t codepoint) const
		{
			if (auto it = glyphs.find(codepoint); it!=glyphs.end())
			{
				return &it->second;
			}
			else
			{
				return nullptr;
			}
		}

		/**
		 * All the textures being used by the glyphs
		 * We can have more than 1, because only load the English glyphs by default. The engine or game then request to load
		 * more glyphs
		 */
		std::vector<SDLUniquePtr<SDL_Texture>> textures;

		/**
		 * Loads all the glyphs required for the specified string
		 */
		bool loadGlyphs(const MTTFFontDefinition& def, std::string_view str);
	};

	MTTFFont(const MTTFFontDefinition* definition);
	~MTTFFont();

	const MTTFFontDefinition& getDefinition()
	{
		return *(static_cast<const MTTFFontDefinition*>(m_definition));
	}

	/**
	 * Loads all the glyphs required for the specified string
	 *
	 * @return Pointer to the instance or nullptr on error
	 */
	const Instance* loadGlyphs(std::string_view str, int ptsize);

	/**
	 * Loads ASCII glyphs.
	 * The following are loaded:
	 * 
	 * 09 (Tab) - A glyph is created that uses the equivalent to 4 spaces
	 * 32 (Space) to 126 (~)
	 * 
	 * @return Pointer to the instance or nullptr on error
	 */
	const Instance* loadASCIIGlyphs(int ptsize);


	/**
	 * Returns the instance with the glyphs for the specified point size.
	 * If not loaded yet, it will load it and create the glyphs for the ASCII characters
	 *
	 * @return Pointer to the instance or nullptr on error
	 */
	const Instance* getPtSize(int ptsize);

  private:

	/**
	 * All the point sizes we loaded
	 */
	std::vector<std::unique_ptr<Instance>> m_instances;
};
MLGE_OBJECT_END(MTTFFont)


NLOHMANN_JSON_SERIALIZE_ENUM( MTTFFontDefinition::RenderMethod, {
	{ MTTFFontDefinition::RenderMethod::Solid, "solid" },
	{ MTTFFontDefinition::RenderMethod::Blended, "blended" }
})

} // namespace mlge

