#include "mlge/Text.h"
#include "mlge/Math.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Render/DebugUtils.h"

#include "utf8.h"
#include "utf8/unchecked.h"

namespace mlge
{

Size TextRendererSettings::calcTextSize(std::string_view text) const
{
	CZ_CHECK(fontInstance);

	const MTTFFont::Glyph* errorGlyph = fontInstance->getGlyph('?');
	if (!errorGlyph)
	{
		return {0, 0};
	}

	// If the font is monospaced, we still need to iterate the codepoints, because a codepoint can be more than
	// 1 byte
	if (fontInstance->monospaced.has_value())
	{
		utf8::unchecked::iterator<const char*> it(text.data());
		int count = 0;
		while(*it)
		{
			count++;
			++it;
		}

		return {count * fontInstance->monospaced.value(), errorGlyph->rect.h};
	}
	else
	{
		Size res = {0, 0};

		utf8::unchecked::iterator<const char*> it(text.data());
		while(*it)
		{
			uint32_t codepoint = *it;
			++it;

			const MTTFFont::Glyph* glyph = fontInstance->getGlyph(codepoint);
			if (!glyph)
			{
				glyph = errorGlyph;
			}

			res.w += glyph->advance;
			res.h = std::max(res.h, glyph->rect.h);
		}

		return res;
	}
}

bool TextRendererSettings::prepareFont()
{
	if (!font)
	{
		return false;
	}
	
	if (!fontInstance)
	{
		fontInstance = font->getPtSize(ptsize);
		if (!fontInstance)
		{
			return false;
		}
	}

	return true;
}

void TextRendererSettings::setFont(MTTFFont& font_)
{
	font = &font_;
	fontInstance = nullptr;
	dirtySize = true;
}

void TextRendererSettings::setPtSize(int ptsize_)
{
	ptsize = ptsize_;
	fontInstance = nullptr;
	dirtySize = true;
}

void TextRendererSettings::renderImpl(std::string_view text, const Size& textSize) const
{
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	const MTTFFont::Glyph* errorGlyph = fontInstance->getGlyph('?');
	if (!errorGlyph)
	{
		return;
	}

	// Save the original clip area
	Rect originalClipRect;
	SDL_RenderGetClipRect(sdlRenderer, &originalClipRect);
	bool hasOriginalClip = SDL_RectEmpty(&originalClipRect) ? false : true;

	if (hasOriginalClip)
	{
		SDL_Rect intersectionRect;
		if (!SDL_IntersectRect(&originalClipRect, &area, &intersectionRect))
		{
			// If the original and our clip rectangles don't intersect, then nothing to render
			return;
		}

		SDL_RenderSetClipRect(sdlRenderer, &intersectionRect);
	}
	else
	{
		SDL_RenderSetClipRect(sdlRenderer, &area);
	}

	Rect dstRect;
	switch(halign)
	{
		case HAlign::Left:
			dstRect.x = area.x;
		break;
		case HAlign::Center:
			dstRect.x = (area.x + area.w/2) - (textSize.w/2);
		break;
		case HAlign::Right:
			dstRect.x = area.right() - textSize.w;
		break;
	}

	switch(valign)
	{
		case VAlign::Top:
			dstRect.y = area.y;
		break;
		case VAlign::Center:
			dstRect.y = (area.y + area.h/2) - (textSize.h/2);
		break;
		case VAlign::Bottom:
			dstRect.y = area.bottom() - textSize.h;
		break;
	}

	utf8::unchecked::iterator<const char*> it(text.data());
	while(*it)
	{
		uint32_t codepoint = *it;
		++it;

		const MTTFFont::Glyph* glyph = fontInstance->getGlyph(codepoint);
		if (!glyph)
		{
			glyph = errorGlyph;
		}

		dstRect.w = glyph->rect.w;
		dstRect.h = glyph->rect.h;

		SDL_SetTextureColorMod(glyph->texture, color.r, color.g, color.b);
		SDL_RenderCopy(sdlRenderer, glyph->texture, &glyph->rect, &dstRect);
		dstRect.x += glyph->advance;
	}

	// Restore the original clip
	if (hasOriginalClip)
	{
		SDL_RenderSetClipRect(sdlRenderer, &originalClipRect);
	}
	else
	{
		SDL_RenderSetClipRect(sdlRenderer, nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////
// MTextRenderComponent
//////////////////////////////////////////////////////////////////////////

bool MTextRenderComponent::defaultConstruct()
{
	m_textRenderer.setAlign(HAlign::Center, VAlign::Center);
	m_textRenderer.setColor(Color::White);
	return Super::defaultConstruct();
}

bool MTextRenderComponent::construct(std::string text)
{
	m_textRenderer.setText(std::move(text));
	return true;
}

void MTextRenderComponent::setFont(ObjectPtr<MTTFFont> font)
{
	m_font = font;
	m_textRenderer.setFont(*m_font);
}

void MTextRenderComponent::setPtSize(int ptsize)
{
	m_textRenderer.setPtSize(ptsize);
}

int MTextRenderComponent::getFontHeight()
{
	return m_textRenderer.getFontHeight();
}

void MTextRenderComponent::setText(std::string_view text)
{
	m_textRenderer.setText(text);
}

void MTextRenderComponent::setAlignment(HAlign halign, VAlign valign)
{
	m_textRenderer.setAlign(halign, valign);
}

void MTextRenderComponent::setColor(const Color& color)
{
	m_textRenderer.setColor(color);
}

void MTextRenderComponent::updateRenderQueue()
{
	if (!m_font)
	{
		return;
	}

	RenderQueue::get().addOp(*this, m_renderGroup);
	#if MLGE_DEBUG
		RenderQueue::get().addOp(*this, m_debugRenderGroup);
	#endif
}


void MTextRenderComponent::render(RenderGroup group)
{
	// Calculate the position relative to the actor
	Point pos = calcFinalPosition();

	//
	// Calculate the text alignment.
	// The component position is the point the text should be relatively aligned to, therefore we do the following:
	// * Calculate the text size
	// * Set the rendering area as double the text area, centered on the component position
	// * The text renderer then used the HAlign/VAlign and the text ends up being aligned relative to the component position.
	Size textSize = m_textRenderer.calcTextSize();
	Rect rect = pos.createRect(textSize.w * 2, textSize.h * 2);
	// If no text to display, then do nothing
	if (rect.isEmpty())
	{
		return;
	}

	if (group == m_renderGroup)
	{
		m_textRenderer.setArea(rect).render();
	}
	#if MLGE_DEBUG
	else if (group == m_debugRenderGroup)
	{
		drawDebugOverlayRect(rect);
	}
	#endif
}

} // namespace mlge

