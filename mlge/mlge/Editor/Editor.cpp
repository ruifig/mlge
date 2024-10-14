#include "mlge/Editor/Editor.h"


#if MLGE_EDITOR

#include "mlge/Editor/GameWindow.h"
#include "mlge/Editor/GameControlBar.h"
#include "mlge/Editor/Console.h"
#include "mlge/Editor/AssetBrowser.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Config.h"
#include "mlge/Engine.h"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "crazygaze/core/Algorithm.h"

namespace mlge
{
	bool gIsGame = false;
} // namespace mlge


namespace mlge::editor
{

Editor::~Editor()
{
	m_imGuiLayer.shutdown();
	m_editorRenderTarget.reset();
}

bool Editor::init()
{
	m_clock.start();

	m_imGuiLayer.init();

	Size windowSize;
	windowSize.w = Config::get().getValueOrDefault<int>("Editor", "resx", 1024);
	windowSize.h = Config::get().getValueOrDefault<int>("Editor", "resy", 768);
	m_editorRenderTarget = std::make_unique<WindowRenderTarget>();
	m_editorRenderTarget->init(windowSize);

	m_windows.emplace(new GameControlBar);

	m_onBeginFrameHandle = Renderer::get().beginFrameDelegate.bind(this, &Editor::onBeginFrame);
	m_onEndFrameHandle = Renderer::get().endFrameDelegate.bind(this, &Editor::onEndFrame);
	m_onGameRenderFinishedHandle = Renderer::get().gameRenderFinishedDelegate.bind(this, &Editor::onGameRenderFinished);
	m_onProcessEventHandle = Engine::get().processEventDelegate.bind(this, &Editor::onProcessEvent);

	SDL_SetWindowTitle(Renderer::get().getSDLWindow(), (std::string(getGameFolderName()) + " Editor").c_str());

	return true;
}

void Editor::requestShutdown()
{
	m_shuttingDown = true;

	visitGamesInfo([](GameInfo& info)
	{
		info.game->requestShutdown();
	});
}

void Editor::onBeginFrame()
{
	m_imGuiLayer.beginFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void Editor::onEndFrame()
{
	m_imGuiLayer.endFrame();
}

void Editor::onGameRenderFinished()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	Renderer::get().setTarget(m_editorRenderTarget.get());
	Renderer::get().clearTarget(Color{
		(Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255)});
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void Editor::onProcessEvent(SDL_Event& evt)
{
	m_imGuiLayer.processEvent(evt);

	if (evt.type == SDL_QUIT)
	{
		requestShutdown();
	}
	else if (
		evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE &&
		evt.window.windowID == SDL_GetWindowID(Renderer::get().getSDLWindow()))
	{
		requestShutdown();
	}
	else if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.scancode == SDL_SCANCODE_LALT)
		{
			if (evt.type == SDL_KEYUP)
			{
				if (anyGameHasFocus())
				{
					CZ_LOG(Log, "Removing focus from game windows");
					setGameFocus(nullptr, false);
				}
			}
		}
	}
	else if (evt.type == SDL_WINDOWEVENT)
	{
		if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			Size s;
			SDL_GetWindowSize(Renderer::get().getSDLWindow(), &s.w, &s.h);

			if (m_editorRenderTarget->getSize() != s)
			{
				m_editorRenderTarget->setSize(s);
				CZ_LOG(Log, "Window resized to {}x{}", s.w, s.h);
				Config::get().setGameValue("Editor", "resx", s.w);
				Config::get().setGameValue("Editor", "resy", s.h);
				Config::get().save();
			}
		}
	}
}

void Editor::tick()
{
	showMenu();
	showImGuiDemoWindow();

	checkExistingWindows();

	float deltaSeconds = m_clock.calcDeltaSeconds();
	for(auto it = m_windows.begin(); it != m_windows.end(); )
	{
		if ((*it)->tick(deltaSeconds))
		{
			++it;
		}
		else
		{
			it = m_windows.erase(it);
		}
	}

	{

		visitGamesInfo([](GameInfo& info)
		{
			MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());

			if (info.stopDeadline.has_value())
			{
				// If the game finished shutting down, or we reached the deadline, then run the final shutdown step
				if (info.game->isShutdownFinished() || std::chrono::high_resolution_clock::now() > info.stopDeadline.value())
				{
					info.game->shutdown();
					info.game.reset();
					info.stopDeadline.reset();
				}
			}
		});

		// Remove any game entries that were shutdown
		cz::remove_if(m_games_, [](const GameInfo& info)
		{
			return info.game == nullptr;
		});
	}

}

void Editor::showMenuFile()
{
	if (ImGui::MenuItem("New")) {}
    ImGui::Separator();
	if (ImGui::MenuItem("Quit"))
	{
		requestShutdown();
	}
}

void Editor::checkExistingWindows()
{
	if (m_showConsole && Console::tryGet() == nullptr)
	{
		m_windows.emplace(std::make_unique<Console>(m_showConsole));
	}

	if (m_showAssetBrowser && AssetBrowser::tryGet() == nullptr)
	{
		m_windows.emplace(std::make_unique<AssetBrowser>(m_showAssetBrowser));
	}
}

void Editor::showMenuWindow()
{
	m_showConsole = Console::tryGet() == nullptr ? false : true;
	if (ImGui::MenuItem("Logging & Console", nullptr, &m_showConsole))
	{
	}

	if (ImGui::MenuItem("Asset Browser", nullptr, &m_showAssetBrowser))
	{
	}
}

void Editor::showImGuiDemoWindow()
{
	if (m_showImGuiDemoWindow)
	{
		ImGui::ShowDemoWindow(&m_showImGuiDemoWindow);
	}
}

void Editor::showMenuHelp()
{
	if (ImGui::MenuItem("ImGui Demo"))
	{
		m_showImGuiDemoWindow = true;
	}
}

void Editor::showMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			showMenuFile();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
			{
			}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X"))
			{
			}
			if (ImGui::MenuItem("Copy", "CTRL+C"))
			{
			}
			if (ImGui::MenuItem("Paste", "CTRL+V"))
			{
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			showMenuWindow();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			showMenuHelp();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

}

bool Editor::startGame(uint32_t count)
{
	if (m_games_.size())
	{
		return false;
	}

	auto startGameImpl = [this]() -> bool
	{
		auto game = createGame();
		MLGE_SET_CURRENT_GAME_INSTANCE(game.get());
		if (game->init())
		{
			uint32_t id = findUnusedGameId();
			m_games_.emplace_back();
			m_games_.back().id = id;
			m_games_.back().game = std::move(game);
			auto gameWindow = std::make_unique<GameWindow>(m_games_.back().game.get(), id);
			m_games_.back().gameWindow = gameWindow.get();

			m_windows.emplace(std::move(gameWindow));
			return true;
		}
		else
		{
			return false;
		}
	};

	while(count--)
	{
		if (!startGameImpl())
		{
			return false;
		}
	}

	return true;
}

// #MULTIPLE_INSTANCES : Refactor this to have the option to stop one or all games.
void Editor::stopGame()
{
	visitGamesInfo([this](GameInfo& info)
	{
		// #RVF : We should probably only destroy the window once the game is confirmed shutdown (so we simulate what happens in non-editor builds)
		//  Delete the Editor window controlling the game
		auto it = m_windows.find(info.gameWindow);
		if (it != m_windows.end())
		{
			m_windows.erase(it);
		}
		info.gameWindow = nullptr;

		// Request the game instance to shutdown
		info.game->requestShutdown();
		int maxShutdownDurationMs = static_cast<int>(Game::get().startShutdown() * 1000.0f);
		// Tick the game until shutdown finishes or the deadline expires
		info.stopDeadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(maxShutdownDurationMs);
	});
}

// #MULTIPLE_INSTANCES : Refactor/remove  this
bool Editor::anyGameHasFocus() const
{
	bool hasFocus = false;
	visitGamesInfo([&](const GameInfo& info)
	{
		if (info.game->hasFocus())
		{
			hasFocus = true;
		}
	});

	return hasFocus;
}

// #MULTIPLE_INSTANCES : Refactor/remove  this
void Editor::setGameFocus(Game* game, bool state)
{
	visitGamesInfo([&](GameInfo& info)
	{
		if (game == nullptr || info.game.get() == game)
		{
			if (state != info.game->hasFocus())	
			{
				info.game->onWindowFocus(state);
			}
		}
	});
	
	if (!state)
	{
		SDL_ShowCursor(true);
	}

	//SDL_SetWindowGrab(Renderer::get().getSDLWindow(), state ? SDL_TRUE : SDL_FALSE);
}

void Editor::addWindow(std::unique_ptr<Window> window)
{
	m_windows.insert(std::move(window));
}

Window* Editor::findWindowByTag(void* tag)
{
	for(auto& w : m_windows)
	{
		if (w->getTag() == tag)
		{
			return w.get();
		}
	}

	return nullptr;
}


} // namespace mlge::Editor

#endif

