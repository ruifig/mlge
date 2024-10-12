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
	MultiCastDelegate<> tickDelegate;

	template<typename TaskFunc>
	void deferToNextTick(TaskFunc&& task)
	{
		m_deferedTasks.emplace(std::forward<TaskFunc>(task));
	}

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

	// #MULTIPLE_STATES : These two should be at the game level, so they get processed with the game instance is set.
	// If the Editor needs to make use of this, then create the same in the `Editor` class.
	cz::SharedQueue<std::function<void()>> m_deferedTasks;
	std::queue<std::function<void()>> m_swapDeferedTasks;

	std::vector<std::unique_ptr<BaseGame>> m_games;
};


template<typename TaskFunc>
void deferTask(TaskFunc&& task)
{
	Engine::get().deferToNextTick(std::forward<TaskFunc>(task));
}

} // namespace mlge