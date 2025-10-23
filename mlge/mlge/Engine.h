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

// #MULTIPLE_INSTANCES : Is this needed ?
#if MLGE_EDITOR
namespace editor
{
	class Editor;
}
#endif


// #MULTIPLE_INSTANCES : Remove this
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

	// #MULTIPLE_INSTANCES : Test if this needs to be a unique_ptr. As-in, check if it's destroyed when:
	// Debug/Development, both in editor or -game mode
	// Release 
	Game* m_game;

	/**
	 * When requesting the game to stop, we set this to shutdown deadline.
	 * If the game doesn't fully stop by then, we kill it.
	 */
	std::optional<std::chrono::high_resolution_clock::time_point> stopDeadline = {};
};

} // namespace mlge

