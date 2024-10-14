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
	void requestShutdown();

	bool isShuttingDown() const
	{
		return m_shuttingDown;
	}

	bool startGame(uint32_t count);
	void stopGame();

	bool anyGameHasFocus() const;
	void setGameFocus(Game* game, bool state);

	void addWindow(std::unique_ptr<Window> window);

	Window* findWindowByTag(void* tag);

  protected:

	friend Engine;

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

	void tick();

	std::set<std::unique_ptr<Window>, details::pointer_comp<Window>> m_windows;
	bool m_shuttingDown = false;
	GameClock m_clock;

	bool m_showConsole = true;
	bool m_showAssetBrowser = true;

	ImGuiLayer m_imGuiLayer;
	std::unique_ptr<RenderTarget> m_editorRenderTarget;
};

} // namespace mlge::editor

#endif

