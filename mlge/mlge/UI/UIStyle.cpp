#include "UIStyle.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

mlge::ObjectPtr<mlge::MTTFFont> MUIStyle::getFont()
{
	if (m_font)
	{
		return m_font;
	}
	else
	{
		return ms_defaultFontRef.getResource();
	}
}

//////////////////////////////////////////////////////////////////////////
// Flat color rendering
//////////////////////////////////////////////////////////////////////////

bool MUIStyleRendererFlat::construct(MUIStyleFlat& outer)
{
	m_outer = &outer;
	return true;
}

Color MUIStyleRendererFlat::getTextColor()
{
	Color c = m_outer->m_textColor;
	return Color(c.r, c.g, c.b, m_enabled ? uint8_t(255) : m_outer->ms_disabledAlpha);
}

void MUIStyleRendererFlat::render(const Rect& rect)
{
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	Color color = m_outer->m_disabledBkgColor;

	auto setColor = [&](const Color& color)
	{
		SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
	};

	if (m_enabled)
	{
		if (m_hover)
		{
			color = m_outer->m_hoverBkgColor;
		}
		else if (m_pressed)
		{
			color = m_outer->m_pressedBkgColor;
		}
		else
		{
			color = m_outer->m_enabledBkgColor;
		}
	}

	SDL_BlendMode originalBlendMode;
	SDL_GetRenderDrawBlendMode(sdlRenderer, &originalBlendMode);
	SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

	setColor(color);
	SDL_RenderFillRect(sdlRenderer, &rect); 

	if (m_outer->m_borderThickness)
	{
		Rect borderRect = rect;
		SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
		setColor(m_outer->m_borderColor);

		int count = m_outer->m_borderThickness;
		while(count--)
		{
			SDL_RenderDrawRect(sdlRenderer, &borderRect);
			borderRect.contract(1);
		}
	}

	SDL_SetRenderDrawBlendMode(sdlRenderer, originalBlendMode);

}

ObjectPtr<MUIStyleRenderer> MUIStyleFlat::createRenderer()
{
	return createObject<MUIStyleRendererFlat>(*this);
}


} // namespace mlge

