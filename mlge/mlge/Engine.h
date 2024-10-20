#pragma once

#include "mlge/Root.h"
#include "mlge/Delegates.h"
#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/SharedQueue.h"
#include "crazygaze/core/ScopeGuard.h"

#include "mlge/Render/DXDebugLayer.h"

namespace mlge
{

class Game;

#if MLGE_EDITOR
namespace editor
{
	class Window;
	class Editor;
}
#endif

/**
 * Bare minimum to have in this header, so we can have std::unique_ptr<BaseGame>
 */
class BaseGame
{
  public:

	BaseGame() = default;
	virtual ~BaseGame() = default;

	CZ_DELETE_COPY_AND_MOVE(BaseGame)

	static BaseGame* setCurrentInstance(BaseGame* instance)
	{
		BaseGame* previous = ms_currentInstance;
		ms_currentInstance = instance;
		return previous;
	}

  protected:
	inline static BaseGame* ms_currentInstance = nullptr;
};

#if MLGE_EDITOR
	/**
	 * This should only be used internally by the Editor code.
	 * There is no need for the game to use this.
	 * It sets the current game instance being processed, so the Editor can have multiple game instances.
	 */
	#define MLGE_SET_CURRENT_GAME_INSTANCE(game)                                 \
		BaseGame* _previousGameInstance = BaseGame::setCurrentInstance(game);    \
		CZ_SCOPE_EXIT{BaseGame::setCurrentInstance(_previousGameInstance); }
#endif

class Engine : public Singleton<Engine>
{
public:
	CZ_DELETE_COPY_AND_MOVE(Engine);

	Engine() = default;
	~Engine();

	bool init(int argc, char* argv[]);
	bool run();

	MultiCastDelegate<SDL_Event&> processEventDelegate;

	struct GameInfo
	{
		uint32_t id;
		std::unique_ptr<BaseGame> game;
		#if MLGE_EDITOR
		editor::Window* editorWindow = nullptr;
		#endif

		/**
		 * When requesting the game to stop, we set this to shutdown deadline.
		 * If the game doesn't fully stop by then, we kill it.
		 */
		std::optional<std::chrono::high_resolution_clock::time_point> stopDeadline = {};
	};

#if MLGE_EDITOR
	uint32_t getGamesCount() const
	{
		uint32_t res = 0;
		for(auto&& info : m_games)
		{
			if (info.game)
			{
				res++;
			}
		}

		return res;
	}

	template<typename Visitor>
	void visitGames(Visitor&& visitor)
	{
		for(GameInfo& info: m_games)
		{
			if (info.game)
			{
				MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
				Game* game = static_cast<Game*>(info.game.get());
				visitor(*game);
			}
		}
	}

	template<typename Visitor>
	void visitGames(Visitor&& visitor) const
	{
		for(const GameInfo& info: m_games)
		{
			if (info.game)
			{
				MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
				Game* game = static_cast<Game*>(info.game.get());
				visitor(*game);
			}
		}
	}

	template<typename Visitor>
	void visitGamesInfo(Visitor&& visitor)
	{
		for(GameInfo& info: m_games)
		{
			if (info.game)
			{
				MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
				visitor(info);
			}
		}
	}

	template<typename Visitor>
	void visitGamesInfo(Visitor&& visitor) const
	{
		for(const GameInfo& info: m_games)
		{
			if (info.game)
			{
				MLGE_SET_CURRENT_GAME_INSTANCE(info.game.get());
				visitor(info);
			}
		}
	}

	friend editor::Editor;
#endif

protected:

	bool initSDL();
	void processEvents();
	void tick();

	/**
	 * What holds all the singletons the game needs
	 * Once the game is destroyed, the singletons are also destroyed
	 */
	std::unique_ptr<Root> m_root;

	/**
	 * This singleton stays here instead of in Root, because it needs to be destroyed only after SDL_Quit
	 */
	DXDebugLayer m_dxDebugLayer;

	bool m_sdlTTFInitialized = false;

	std::array<GameInfo, MLGE_EDITOR ? 10 : 1> m_games;

	uint32_t findUnusedGameId() const
	{
		uint32_t id = 0;
		bool used = true;
		while(used)
		{
			used = false;
			for(const GameInfo& info : m_games)
			{
				if (info.game && info.id == id)
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

	/**
	* Creates a new game and adds it to the games list.
	* Returns the GameInfo if the game was added, nullptr if it failed.
	*/
	GameInfo* createNewGame();
};


} // namespace mlge