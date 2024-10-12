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

	int getGamesCount() const
	{
		return static_cast<int>(m_games_.size());
	}

	Game& getGameAtIndex(int idx)
	{
		return *m_games_[static_cast<size_t>(idx)].game;
	}

	template<typename Visitor>
	void visitGames(Visitor&& visitor)
	{
		for(GameInfo& info: m_games_)
		{
			MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
			visitor(*info.game);
		}
	}

	template<typename Visitor>
	void visitGames(Visitor&& visitor) const
	{
		for(const GameInfo& info: m_games_)
		{
			MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
			visitor(*info.game);
		}
	}

	template<typename Visitor>
	void visitGamesInfo(Visitor&& visitor)
	{
		for(GameInfo& info: m_games_)
		{
			MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
			visitor(info);
		}
	}

	template<typename Visitor>
	void visitGamesInfo(Visitor&& visitor) const
	{
		for(const GameInfo& info: m_games_)
		{
			MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
			visitor(info);
		}
	}

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

	struct GameInfo
	{
		uint32_t id = 0;
		std::unique_ptr<Game> game;
		// Editor window used to render the game
		Window* gameWindow;

		/**
		 * When requesting the game to stop, we set this to shutdown deadline.
		 * If the game doesn't fully stop by then, we kill it.
		 */
		std::optional<std::chrono::high_resolution_clock::time_point> stopDeadline;
	};

	std::vector<GameInfo> m_games_;
	uint32_t findUnusedGameId() const
	{
		uint32_t id = 0;
		bool used = true;
		while(used)
		{
			used = false;
			for(const GameInfo& info : m_games_)
			{
				if (info.id == id)
				{
					used = true;
					break;
				}
			}

			if (used)
			{
				id++;
			}
			else
			{
				break;
			}
		}

		return id;
	}



	bool m_showConsole = true;
	bool m_showAssetBrowser = true;

	ImGuiLayer m_imGuiLayer;
	std::unique_ptr<RenderTarget> m_editorRenderTarget;
};

} // namespace mlge::editor

#endif

