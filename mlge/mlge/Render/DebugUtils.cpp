#include "mlge/Render/DebugUtils.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

void drawDebugOverlayRect(const Rect& area)
{
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	// SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(sdlRenderer, Color::Pink.r, Color::Pink.g, Color::Pink.b, 255);
	SDL_RenderDrawRect(sdlRenderer, &area);
	SDL_SetRenderDrawColor(sdlRenderer, Color::Pink.r / 2u, Color::Pink.g / 2u, Color::Pink.b / 2u, 255);
	SDL_RenderDrawLine(sdlRenderer, area.x, area.y + area.h / 2, area.x + area.w, area.y + area.h / 2);
	SDL_RenderDrawLine(sdlRenderer, area.x + area.w / 2, area.y, area.x + area.w / 2, area.y + area.h);
}

} // namespace mlge

