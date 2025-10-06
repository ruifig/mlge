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

namespace mlge
{

Engine::~Engine()
{
	// NOTE: using "visit" so each game is set as "current" when we delete it.
	visitGamesInfo([](GameInfo& info)
	{
		info.game = nullptr;
	});

	std::generate(m_games.begin(), m_games.end(), []{ return GameInfo(); });

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

		// #MULTIPLE_INSTANCES: Fix this. I'm setting the game as current because at the time of writing, there is code in the game(s) that subscribes
		// to this

		{
			MLGE_SET_CURRENT_GAME_INSTANCE(tryGetFirstGame());
			processEventDelegate.broadcast(evt);
		}

		if (evt.type == SDL_QUIT)
		{
			if (gIsGame)
			{
				visitGames([](Game& game)
				{
					game.requestExpectedShutdown();
				});
			}
		}
		else if (evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(Renderer::get().getSDLWindow()))
			{
				if (gIsGame)
				{
					visitGames([](Game& game)
					{
						game.requestExpectedShutdown();
					});
				}
			}

			if (gIsGame)
			{
				visitGames([&evt](Game& game)
				{
					if (evt.window.event == SDL_WINDOWEVENT_ENTER)
					{
						game.onWindowEnter(true);
					}
					if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
					{
						game.onWindowEnter(false);
					}
					else if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						game.onWindowResized({evt.window.data1, evt.window.data2});
					}
				});
			}

			if (gIsGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				visitGames([](Game& game)
				{
					game.onWindowFocus(true);
				});
			}
			else if (gIsGame && evt.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				visitGames([](Game& game)
				{
					game.onWindowFocus(false);
				});
			}

		}
		else if (gIsGame && evt.type == SDL_MOUSEMOTION)
		{
			Game::MouseMotionEvent gameEvt;
			gameEvt.pos = {evt.motion.x, evt.motion.y};
			gameEvt.rel = {evt.motion.xrel, evt.motion.yrel};
			visitGames([&gameEvt](Game& game)
			{
				game.onMouseMotion(gameEvt);
			});
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

Engine::GameInfo* Engine::createNewGame()
{
	// Find a free slot
	GameInfo* info = nullptr;
	for(GameInfo& i  : m_games)
	{
		if (i.game == nullptr)
		{
			info = &i;
			break;
		}
	}

	// No free game slot found
	if (!info)
	{
		CZ_LOG(Error, "No available game slot.");
		return nullptr;
	}

	std::unique_ptr<Game> game = createGame();

	{
		MLGE_SET_CURRENT_GAME_INSTANCE(game.get());
		if (!game->init())
		{
			game = nullptr;
			return nullptr;
		}
	}

	*info = {};
	info->id = findUnusedGameId();
	info->game = std::move(game);
	return info;
}

bool Engine::init(int argc, char* argv[])
{
	// Set the working directory to the executable's folder
	fs::current_path(getProcessPath());
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
		if (!createNewGame())
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


	visitGames([](Game& game)
	{
		game.gameClockTick();
	});


	// Reset slots that don't have a game
	for(GameInfo& info : m_games)
	{
		if (info.game == nullptr)
		{
			info = {};
		}
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
		visitGames([&](Game& game)
		{
			shuttingDown &= game.isShuttingDown();
		});

	#if MLGE_EDITOR
		if (editor::Editor::tryGet())
		{
			shuttingDown &= editor::Editor::get().isShuttingDown();
		}
	#endif

		visitGames([&](Game& game)
		{
			PerformanceStats::get().tick();
		});

	} while(shuttingDown == false);

	CZ_LOG(Log, "Starting shutdown...");


	// #MULTIPLE_INSTANCES: Remove this:
	BaseGame* game = Game::tryGet() ? Game::tryGet() : Engine::get().tryGetFirstGame();
	MLGE_SET_CURRENT_GAME_INSTANCE(game);

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

	return Game::get().getShutdownValue();
}

} // namespace mlge

