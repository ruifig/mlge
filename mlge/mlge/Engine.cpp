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
#include "crazygaze/core/Algorithm.h"

#if MLGE_EDITOR
	#include "mlge/Editor/Editor.h"
#endif

CZ_DEFINE_LOG_CATEGORY(Editor)

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
		CZ_LOG(Main, Fatal, "Could not initialize SDL. ec={}", SDL_GetError());
		return false;
	}

	// For ImgUI. Not sure what this does.
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	// SDL_SetWindowGrab will grab both the mouse and keyboard
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1"); 

	if (TTF_Init() < 0)
	{
		CZ_LOG(Main, Fatal, "Could not initialize SDL TTF. ec={}", TTF_GetError());
		return false;
	}

	m_sdlTTFInitialized = true;

	return true;
}

void Engine::processEvents()
{
	MLGE_PROFILE_SCOPE(mlge_Engine_processInput);

	SDL_Event evt;

	// Note that gIsGame will be get in Editor builds when we pass -game
	// The point of this bool
	// - If true, then we are in a situation that we are an actual game (eithe Release build, or running with -game)
	// - If false, then we are runnig in editor mode
	//		- Even if the game is running inside the editor, this is still false (intentional), so that this functio can skip
	//		  things that are the Editor's responsability
	bool hasGame = gIsGame && Game::tryGet();

	while(SDL_PollEvent(&evt))
	{
		processEventDelegate.broadcast(evt);

		if (evt.type == SDL_QUIT)
		{
			if (hasGame)
			{
				Game::get().requestExpectedShutdown();
			}
		}
		else if (evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(Renderer::get().getSDLWindow()))
			{
				if (hasGame)
				{
					Game::get().requestExpectedShutdown();
				}
			}

			if (hasGame)
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

			if (hasGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				Game::get().onWindowFocus(true);
			}
			else if (hasGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				Game::get().onWindowFocus(false);
			}
		}
		else if (hasGame && evt.type == SDL_MOUSEMOTION)
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
		LogLevel level = logLevelFromString(levelStr);
		setLogLevel(level);
	}
}

bool Engine::init(int argc, char* argv[])
{
	// Set the working directory to the executable's folder
	fs::current_path(getProcessPath());
	m_root = Root::create();

	// This needs to be initialized before Root, so the other singletons can query the command line
	if (!CommandLine::get().init(argc, argv))
	{
		CZ_LOG(Main, Error, "Unexpected things in the command line.");
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
	#if MLGE_EDITOR
		if (!gIsGame)
		{
			editor::Editor::get().tick();
		}
	#endif

	if (Game::tryGet())
	{
		Game::get().gameClockTick();
	};
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

	do
	{
		shuttingDown = true;

		MLGE_PROFILE_SCOPE(mlge_Engine_run);

		fpsLimiter.tick();

		{
			if (Game::tryGet())
			{
				PerformanceStats::get().stat_Tick_Start();
			}

			Renderer::get().beginFrame();
			processEvents();
			tick();

			if (Game::tryGet())
			{
				PerformanceStats::get().stat_Tick_End();
			}
		}

		Renderer::get().render();

		// We initiate shutdown if both the game and editor want to shutdown
		shuttingDown &= Game::tryGet() && Game::get().isShuttingDown();

	#if MLGE_EDITOR
		if (editor::Editor::tryGet())
		{
			shuttingDown &= editor::Editor::get().isShuttingDown();
		}
	#endif

		if (Game::tryGet())
		{
			PerformanceStats::get().tick();
		}

	} while(shuttingDown == false);

	CZ_LOG(Main, Log, "Starting shutdown...");


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
				CZ_LOG(Main, Warning, "Shutdown deadline expired. Forcing shutdown.")
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

	return Game::get().getShutdownValue();
}

} // namespace mlge

