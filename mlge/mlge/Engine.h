#pragma once

#include "mlge/Root.h"
#include "mlge/Delegates.h"
#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/SharedQueue.h"

#include "mlge/Render/DXDebugLayer.h"

namespace mlge
{

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

	cz::SharedQueue<std::function<void()>> m_deferedTasks;
	std::queue<std::function<void()>> m_swapDeferedTasks;

	// This is only created if running a non-editor build or an editor build with -game.
	// NOTE: Using a naked pointer because using a std::unique_ptr would require a dependency on the header
	Game* m_game = nullptr;
};


template<typename TaskFunc>
void deferTask(TaskFunc&& task)
{
	Engine::get().deferToNextTick(std::forward<TaskFunc>(task));
}

} // namespace mlge