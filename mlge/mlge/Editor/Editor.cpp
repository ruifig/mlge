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
	m_onTickHandle = Engine::get().tickDelegate.bind(this, &Editor::onTick);

	SDL_SetWindowTitle(Renderer::get().getSDLWindow(), (std::string(getGameFolderName()) + " Editor").c_str());

	return true;
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
			if (evt.type == SDL_KEYUP && gameHasFocus())
			{
				CZ_LOG(Log, "Removing focus from game window");
				setGameFocus(false);
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

void Editor::onTick()
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

	if (m_stopDeadline.has_value())
	{
		// If the game finished shutting down, or we reached the deadline, then run the final shutdown step
		if (m_game->isShutdownFinished() || std::chrono::high_resolution_clock::now() > m_stopDeadline.value())
		{
			m_game->shutdown();
			m_game.reset();
			m_stopDeadline.reset();
		}
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

bool Editor::startGame()
{
	if (m_game)
	{
		return false;
	}

	auto game = createGame();
	if (game->init())
	{
		m_game = std::move(game);
		auto gameWindow = std::make_unique<GameWindow>();
		m_gameWindow = gameWindow.get();
		m_windows.emplace(std::move(gameWindow));
		return true;
	}
	else
	{
		return false;
	}
}

bool Editor::stopGame()
{
	if (m_gameWindow)
	{
		auto it = m_windows.find(m_gameWindow);
		if (it != m_windows.end())
		{
			m_windows.erase(it);
		}
		m_gameWindow = nullptr;
	}

	if (m_game)
	{
		m_game->requestShutdown();
		int maxShutdownDurationMs = static_cast<int>(Game::get().startShutdown() * 1000.0f);
		// Tick the game until shutdown finishes or the deadline expires
		m_stopDeadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(maxShutdownDurationMs);
		return true;
	}
	else
	{
		return false;
	}
}

void Editor::setGameFocus(bool state)
{
	if (!m_game)
	{
		return;
	}

	if (state != m_game->hasFocus())
	{
		m_game->onFocusChanged(state);
	}

	SDL_SetWindowGrab(Renderer::get().getSDLWindow(), state ? SDL_TRUE : SDL_FALSE);
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

