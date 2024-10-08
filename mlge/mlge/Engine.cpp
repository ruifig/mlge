#include "mlge/Engine.h"
#include "mlge/Profiler.h"
#include "mlge/Game.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Render/DXDebugLayer.h"
#include "mlge/Resource/Resource.h"
#include "mlge/Config.h"
#include "mlge/PerformanceStats.h"

#include "crazygaze/core/ScopeGuard.h"
#include "crazygaze/core/CommandLine.h"

#if MLGE_EDITOR
	#include "mlge/Editor/Editor.h"
#endif

namespace mlge
{

Engine::~Engine()
{
	if (m_game)
	{
		delete m_game;
		m_game = nullptr;
	}

	// Explicit delete, so everything gets destroyed before shutting down SDL
	m_root = nullptr;

	if (m_sdlTTFInitialized)
	{
		TTF_Quit();
	}

	// This needs to be after Renderer is shutdown, so it doesn't show SDL references as leaks
	DXDebugLayer::get().shutdown();

	SDL_Quit();
}

bool Engine::initSDL()
{
	MLGE_PROFILE_SCOPE(mlge_Engine_initSDL);

	if (!DXDebugLayer::get().init())
	{
		return false;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		CZ_LOG(Fatal, "Could not initialize SDL. ec={}", SDL_GetError());
		return false;
	}

	// For ImgUI. Not sure what this does.
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	// SDL_SetWindowGrab will grab both the mouse and keyboard
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); 

	if (TTF_Init() < 0)
	{
		CZ_LOG(Fatal, "Could not initialize SDL TTF. ec={}", TTF_GetError());
		return false;
	}

	m_sdlTTFInitialized = true;

	return true;
}

void Engine::processEvents()
{
	MLGE_PROFILE_SCOPE(mlge_Engine_processInput);

	SDL_Event evt;

	while(SDL_PollEvent(&evt))
	{
		processEventDelegate.broadcast(evt);

		if (Game::tryGet())
		{
			Game::get().processEvent(evt);
		}

		if (evt.type == SDL_QUIT)
		{
			if (Game::tryGet())
			{
				Game::get().requestShutdown();
			}
		}
		else if (evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(Renderer::get().getSDLWindow()))
			{
				if (Game::tryGet())
				{
					Game::get().requestShutdown();
				}
			}

			// #RVF : Once I add support for multiple games in the editor, these needs to be forward these to the right Game
			// instance. In short, from the windowID, it should get the game instance, and broadcast the event of that one. 
			if (gIsGame && Game::tryGet())
			{
				if (evt.window.event == SDL_WINDOWEVENT_ENTER)
				{
					Game::get().onWindowEnter(true);
				}
				if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
				{
					Game::get().onWindowEnter(false);
				}
				else if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					Game::get().onWindowResized({evt.window.data1, evt.window.data2});
				}
			}

			if (gIsGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				Game::get().onWindowFocus(true);
			}
			else if (gIsGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				Game::get().onWindowFocus(false);
			}

		}
		else if (gIsGame && evt.type == SDL_MOUSEMOTION)
		{
			Game::MouseMotionEvent gameEvt;
			gameEvt.pos = {evt.motion.x, evt.motion.y};
			gameEvt.rel = {evt.motion.xrel, evt.motion.yrel};
			Game::get().onMouseMotion(gameEvt);
		}

	}

}

namespace details
{
	void applyLogLevels()
	{
		std::string levelStr = Config::get().getValueOrDefault<std::string>("Engine", "loglevel", to_string(compileTimeMaxLogLevel));
		currMaxLogLevel = logLevelFromString(levelStr);
	}
}

bool Engine::init(int argc, char* argv[])
{
	m_root = Root::create();

	// This needs to be initialized before Root, so the other singletons can query the command line
	if (!CommandLine::get().init(argc, argv))
	{
		CZ_LOG(Error, "Unexpected things in the command line.");
	}

	if (!initSDL())
	{
		return false;
	}

	if (!m_root->init())
	{
		return false;
	}

	details::applyLogLevels();

	if (!ResourceManager::get().loadDefinitions())
	{
		return false;
	}

	if (gIsGame)
	{
		m_game = createGame().release();
		if (!Game::get().init())
		{
			return false;
		}
	}

	return true;
}

void Engine::tick()
{
	if (m_deferedTasks.popAll(m_swapDeferedTasks))
	{
		while(m_swapDeferedTasks.size())
		{
			// Note the double ()(). Intentional.
			m_swapDeferedTasks.front()();
			m_swapDeferedTasks.pop();
		}
	}

	tickDelegate.broadcast();

	if (Game::tryGet())
	{
		Game::get().gameClockTick();
	}
}


namespace
{

	/**
	 * Limits the frequency of a loop (e.g: game framerate)
	 *
	 * Based on https://stackoverflow.com/questions/38730273/how-to-limit-fps-in-a-loop-with-c
	 */
	class FPSLimiter
	{
	  public:

		using Clock = std::chrono::high_resolution_clock;

		/**
		 * @param maxFps If 0, no limit is imposed.
		 */
		explicit FPSLimiter(int maxFps)
			: m_maxFps(maxFps)
		{
			if (m_maxFps == 0)
			{
				return;
			}

			m_tsA = Clock::now();
			m_tsB = m_tsA;
			m_msPerFrame = 1000.0f / static_cast<float>(m_maxFps);
		}

		void tick()
		{
			m_tsA = Clock::now();
			m_lastWorkTime = m_tsA - m_tsB;

			if (m_maxFps != 0)
			{
				if (m_lastWorkTime.count() < m_msPerFrame)
				{
					std::chrono::duration<double, std::milli> delta_ms(m_msPerFrame - m_lastWorkTime.count());
					auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
					std::this_thread::sleep_for(delta_ms_duration);
				}
			}

			m_tsB = Clock::now();
		}

		float getLastWorkTimeMs() const
		{
			return static_cast<float>(m_lastWorkTime.count());
		}

	  private:

		Clock::time_point m_tsA;
		Clock::time_point m_tsB;

		// Time spent over the last frame, excluding the frame limting
		std::chrono::duration<double, std::milli> m_lastWorkTime = {};

		int m_maxFps;
		float m_msPerFrame;
		
	};

}

bool Engine::run()
{
	CZ_SCOPE_EXIT
	{
		Renderer::get().shutdown();
	};

	bool shuttingDown;

	int maxFps = Config::get().getValueOrDefault("Engine", "maxfps", 0);
	FPSLimiter fpsLimiter(maxFps);
	PerformanceStats performanceStats;
	performanceStats.setEnabled(true);

	do
	{
		shuttingDown = true;

		MLGE_PROFILE_SCOPE(mlge_Engine_run);

		fpsLimiter.tick();

		{
			performanceStats.stat_Tick_Start();

			Renderer::get().beginFrame();
			processEvents();
			tick();

			performanceStats.stat_Tick_End();
		}

		Renderer::get().render();

		// We initiate shutdown if both the game and editor want to shutdown
		if (Game::tryGet())
		{
			shuttingDown = Game::get().isShuttingDown();
		}

	#if MLGE_EDITOR
		if (editor::Editor::tryGet())
		{
			shuttingDown &= editor::Editor::get().isShuttingDown();
		}
	#endif

		performanceStats.tick();

	} while(shuttingDown == false);

	CZ_LOG(Log, "Starting shutdown...");

	// Start the shutdown.
	if (Game::tryGet())
	{
		int maxShutdownDurationSeconds = static_cast<int>(Game::get().startShutdown());

		// Tick the game until shutdown finishes or the deadline expires
		auto shutdownTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(maxShutdownDurationSeconds);

		while(true)
		{
			auto currentTime = std::chrono::high_resolution_clock::now();

			if (currentTime >= shutdownTime)
			{
				CZ_LOG(Warning, "Shutdown deadline expired. Forcing shutdown.")
				break;
			}

			if (Game::get().isShutdownFinished())
			{
				break;
			}

			Renderer::get().beginFrame();
			tick();
			Renderer::get().render();
		}

		Game::get().shutdown();
	}


	return true;
}

} // namespace mlge

