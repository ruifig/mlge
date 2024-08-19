#pragma once

#if MLGE_EDITOR

#include "mlge/Common.h"
#include "mlge/Editor/Window.h"
#include "mlge/Editor/ImGuiLayer.h"
#include "mlge/GameClock.h"
#include "mlge/Game.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Delegates.h"

#include "crazygaze/core/Singleton.h"

namespace mlge::editor
{

class Editor : public Singleton<Editor>
{
  public:

	~Editor();

	bool init();
	void requestShutdown()
	{
		m_shuttingDown = true;
		if (m_game)
		{
			m_game->requestShutdown();
		}
	}

	bool isShuttingDown() const
	{
		return m_shuttingDown;
	}

	bool startGame();
	bool stopGame();

	/**
	 * Returns true if the game is running and has input focus
	 */
	bool gameHasFocus() const
	{
		return m_game && m_game->hasFocus();
	}

	/**
	 * Returns true if the game is running (with focus or not)
	 */
	bool gameIsRunning() const
	{
		return m_game.get() ? true : false;
	}

	void setGameFocus(bool state);

	void addWindow(std::unique_ptr<Window> window);

	Window* findWindowByTag(void* tag);

  protected:

	void showMenu();
	void showMenuWindow();
	void showMenuFile();
	void showMenuHelp();
	void checkExistingWindows();

	bool m_showImGuiDemoWindow = false;
	void showImGuiDemoWindow();

	void onBeginFrame();
	DelegateHandle m_onBeginFrameHandle;
	void onEndFrame();
	DelegateHandle m_onEndFrameHandle;
	void onGameRenderFinished();
	DelegateHandle m_onGameRenderFinishedHandle;
	void onProcessEvent(SDL_Event& evt);
	DelegateHandle m_onProcessEventHandle;
	void onTick();
	DelegateHandle m_onTickHandle;

	std::set<std::unique_ptr<Window>, details::pointer_comp<Window>> m_windows;
	bool m_shuttingDown = false;
	GameClock m_clock;

	std::unique_ptr<Game> m_game;
	Window* m_gameWindow = nullptr;

	bool m_showConsole = true;
	bool m_showAssetBrowser = true;

	/**
	 * When requesting the game to stop, we set this to shutdown deadline.
	 * If the game doesn't fully stop by then, we kill it.
	 */
	std::optional<std::chrono::high_resolution_clock::time_point> m_stopDeadline;

	ImGuiLayer m_imGuiLayer;
	std::unique_ptr<RenderTarget> m_editorRenderTarget;
};

} // namespace mlge::editor

#endif

