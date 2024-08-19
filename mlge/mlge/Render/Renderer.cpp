#include "mlge/Render/Renderer.h"
#include "mlge/Game.h"
#include "mlge/Profiler.h"
#include "mlge/Config.h"
#include "mlge/Paths.h"

#include "mlge/Render/RenderQueue.h"
#include "mlge/Render/DXDebugLayer.h"

namespace mlge
{

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	// Doing manual resets of the smart pointers, to force the order of destruction in case the order in the headers is not
	// exactly right
	m_sdlRenderer.reset();
	m_sdlWindow.reset();
}

bool Renderer::init()
{
	// Create SDL window
	{
		bool fullscreen = CommandLine::get().has("fullscreen");
		SDL_WindowFlags windowFlags;
		if (fullscreen)
		{
			windowFlags = (SDL_WindowFlags)(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
		}
		else
		{
			windowFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		}


		Size windowSize;
		windowSize.w = Config::get().getValueOrDefault<int>("Engine", "resx", 0);
		windowSize.h = Config::get().getValueOrDefault<int>("Engine", "resy", 0);

		if (windowSize.w == 0 || windowSize.h == 0)
		{
			windowSize.w = 1024;
			windowSize.h = 768;

			Config::get().setGameValue("Engine", "resx", windowSize.w);
			Config::get().setGameValue("Engine", "resy", windowSize.h);
			Config::get().save();
		}

		m_sdlWindow.reset(SDL_CreateWindow(
			getGameFolderName().data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize.w, windowSize.h, windowFlags));
		if (!m_sdlWindow)
		{
			CZ_LOG(Fatal, "Could not create SDL window. ec={}", SDL_GetError());
			return false;
		}
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	int rendererIndex = -1;

	auto renderApi = Config::get().getValueOrDefault<std::string>("Engine", "renderapi", "direct3d11");
	CZ_LOG(Log, "SDL available renderers:")
	for(int i=0; i<SDL_GetNumRenderDrivers(); i++)
	{
		SDL_RendererInfo info{};
		SDL_GetRenderDriverInfo(i, &info);
		if (std::string_view(info.name) == renderApi)
		{
			rendererIndex = i;
		}
		CZ_LOG(Log, "    {}", info.name);
	}

	uint32_t rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	SDL_SetHint(SDL_HINT_RENDER_DIRECT3D_THREADSAFE, "1");
	m_sdlRenderer.reset(SDL_CreateRenderer(m_sdlWindow.get(), rendererIndex, rendererFlags));
	if (!m_sdlRenderer)
	{
		CZ_LOG(Fatal, "Could not create SDL renderer. ec={}", SDL_GetError());
		return false;
	}

	// Log renderer info
	{
		SDL_RendererInfo info{};
		SDL_GetRendererInfo(m_sdlRenderer.get(), &info);
		CZ_LOG(Log, "RendererInfo: Name={}, flags={}", info.name, info.flags);
	}

	DXDebugLayer::get().setD3DDebug(*m_sdlRenderer);

	return true;
}

void Renderer::setTarget(RenderTarget* target)
{
	m_renderTarget = target;
	CZ_VERIFY(SDL_SetRenderTarget(m_sdlRenderer.get(), target->getTexture()) == 0);
}

void Renderer::clearTarget(Color clearColor)
{
	SDL_SetRenderDrawColor(m_sdlRenderer.get(), clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	SDL_RenderClear(m_sdlRenderer.get());
}

void Renderer::beginFrame()
{
	beginFrameDelegate.broadcast();
}

void Renderer::shutdown()
{
}

void Renderer::draw()
{
	MLGE_PROFILE_SCOPE(mlge_Renderer_draw);

	endFrameDelegate.broadcast();
	RenderQueue::get().render();
	gameRenderFinishedDelegate.broadcast();
}

void Renderer::render()
{
	MLGE_PROFILE_SCOPE(mlge_Renderer_render);

	draw();

	{
		MLGE_PROFILE_SCOPE(mlge_Renderer_SDL_RenderPresent);
		SDL_RenderPresent(m_sdlRenderer.get());
	}

	m_frameNumber++;
}


} // namespace mlge

