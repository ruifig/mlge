#pragma once

#include "mlge/Root.h"
#include "mlge/Delegates.h"
#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/SharedQueue.h"

#include "mlge/Render/DXDebugLayer.h"

namespace mlge
{

/**
 * Bare mininum to have in this header, so we can have std::unique_ptr<BaseGame>
 */
class BaseGame
{
  public:

	BaseGame() = default;
	virtual ~BaseGame() = default;

	CZ_DELETE_COPY_AND_MOVE(BaseGame)
};

class Game;

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


	std::vector<std::unique_ptr<BaseGame>> m_games;
};


} // namespace mlge