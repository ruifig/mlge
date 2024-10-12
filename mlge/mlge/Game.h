#pragma once

#include "mlge/Common.h"
#include "mlge/Level.h"
#include "mlge/GameClock.h"
#include "mlge/Delegates.h"
#include "mlge/Paths.h"
#include "mlge/Engine.h"

#include "crazygaze/core/Singleton.h"
#include "crazygaze/core/ScopeGuard.h"

/**
 * A game should use this macro in a single CPP file
 */
#define MLGE_IMPLEMENT_GAME(GameClass, GameFolderName)      \
	std::unique_ptr<Game> createGame()                      \
	{                                                       \
		return std::make_unique<GameClass>();               \
	}                                                       \
	                                                        \
	GameClass& getGame()                                    \
	{                                                       \
		return static_cast<GameClass&>(Game::get());        \
	}                                                       \
	                                                        \
	std::string_view getGameFolderName()                    \
	{                                                       \
		return GameFolderName;                              \
	}                                                       \
	                                                        \
	fs::path getGameRelativePath()                          \
	{                                                       \
		static fs::path rel = fs::relative(fs::path(__FILE__).parent_path().parent_path(),  getBuildRoot()); \
		return rel;                              \
	}


#define MLGE_DECLARE_GAME(GameClass)   \
	GameClass& getGame();              \
	fs::path getGameRelativePath();

#if MLGE_EDITOR
	/**
	 * This should only be used internally by the Editor code.
	 * There is no need for the game to use this.
	 * It sets the current game instance being processed, so the Editor can have multiple game instances.
	 */
	#define MLGE_SET_CURRENT_GAME_INSTANCE(game)                            \
		Game* _previousGameInstance = Game::setCurrentInstance(game);       \
		CZ_SCOPE_EXIT{Game::setCurrentInstance(_previousGameInstance); }
#endif


namespace mlge
{

class MLevel;
class RenderTarget;
class UIManager;
class RenderQueue;
class PerformanceStats;

/**
 * Defers a task for execution on the next tick of the current game
 */
template<typename TaskFunc>
void deferTask(TaskFunc&& task)
{
	if (Game* game = Game::tryGet())
	{
		game->deferToNextTick(std::forward<TaskFunc>(task));
	}
	else
	{
		CZ_CHECK(false);
	}
}


#if MLGE_EDITOR
namespace editor
{
	class Editor;
	class GameWindow;
}
#endif

class Game : public BaseGame
{
  public:

	Game(std::string_view name);
	virtual ~Game();

	/**
	 * Gets the game instance currently being processed.
	 * It asserts if there isn't an instance.
	 * If the caller code needs to check if an instance exists, it should use tryGet instead.
	 */
	static Game& get()
	{
		CZ_CHECK(ms_currentInstance);
		return *ms_currentInstance;
	}

	/**
	 * Tries getting the game instance currently being processed
	 */
	static Game* tryGet()
	{
		return ms_currentInstance;
	}

	/**
	 * Returns the game name
	 */
	const std::string& getName() const
	{
		return m_name;
	}

	/**
	 * Returns a string in the form:
	 * "<GAME NAME> v<GAME VERSION>, GitHash:<GIT HASH>, Build type:<BUILD TYPE>, Build Timestamp:<BUILD UTC TIME> UTC"
	 */
	const std::string& getBuildInfo() const;

	/**
	 * Called once at the game start
	 *
	 * NOTE: A derived class should call the parent's function first.
	 * Derived classes can implement this to add game specific initialization.
	 *
	 * @return True on success, false on error. 
	 * 
	 */
	virtual bool init();

	/**
	 * Called during the game loop
	 */
	virtual void tick(float deltaSeconds);

	/**
	 * Causes the game to initiate shutdown. 
	 * Any game system(s) that want to cause the game to shutdown gracefully should call this
	 */
	 virtual void requestShutdown();

	/**
	 * Called when any systems in the game or engine request a shutdown (by calling requestShutdown())
	 * NOTE: A derived class should call the parent's function first.
	 *
	 * If the game has some task that needs to be done at shutdown and still requires ticking, it should do the following:
	 * - Override this function, start those tasks and return an appropriate maximum wait time.
	 *
	 * @return How long in seconds to keep ticking the game before forcing the shutdown
	 */
	virtual float startShutdown();

	/**
	 * This is called during the shutdown process.
	 * If the game has work to finish during shutdown, it should override this function and return true when the work is finished. 
	 *
	 * The game will continue to be ticked until this returns true or the deadline the game returned in startShutdown expires.
	 */
	virtual bool isShutdownFinished();

	/**
	 * Called right before the game exits
	 *
	 * NOTE: A derived class should call the parent's function first.
	 */
	virtual void shutdown();

	/**
	 * Called when the mouse cursor enters or leaves the window
	 */
	virtual void onWindowEnter(bool entered);
	/**
	 * Called when the window is resized
	 */
	virtual void onWindowResized(const Size& size);


	/**
	 * Called when the window gains or loses focus
	 */
	virtual void onWindowFocus(bool focus);

	/**
	 * Gets the current level
	 * If a level is not set yet, it creates an empty "Level" instance
	 */
	MLevel& getLevel();

	bool isShuttingDown() const
	{
		return m_shuttingDown;
	}

	double getGameTimeSecs() const
	{
		return m_clock.getGameTimeSecs();
	}

	double getWallTimeSecs() const
	{
		return m_clock.getWallTimeSecs();
	}

	bool isGameClockPaused() const
	{
		return m_clock.isPaused();
	}

	void pauseGameClock()
	{
		m_clock.pause();
	}

	void resumeGameClock()
	{
		m_clock.resume();
	}

	bool hasFocus() const
	{
		return m_hasFocus;
	}

	RenderTarget& getRenderTarget()
	{
		return *m_renderTarget;
	}

	MultiCastDelegate<Size> windowResizedDelegate;
	// Broadcast when the mouse enters or leaves the window
	MultiCastDelegate<bool> windowEnterDelegate;
	MultiCastDelegate<bool> windowFocus;

	struct MouseMotionEvent
	{
		// Mouse position within the window
		Point pos;
		// Relative mouse movement
		Point rel;
	};

	MultiCastDelegate<const MouseMotionEvent&> mouseMotionDelegate;

	/**
	 * Called when a mouse movement occurs
	 */
	virtual void onMouseMotion(const MouseMotionEvent& evt);

	RenderQueue& getRenderQueue()
	{
		return *m_renderQueue;
	}

	PerformanceStats& getPerformanceStats()
	{
		return *m_performanceStats;
	}

	template<typename TaskFunc>
	void deferToNextTick(TaskFunc&& task)
	{
		m_deferedTasks.emplace(std::forward<TaskFunc>(task));
	}

  protected:

	// The order is important, because we need a specific destruction order (the C++ standards guarantees the objects are
	// destroyed in the reverse order)
	std::unique_ptr<RenderQueue> m_renderQueue;
	std::unique_ptr<UIManager> m_ui;
	std::unique_ptr<PerformanceStats> m_performanceStats;

	/**
	 * How long the game loop will wait for the shutdown before forcing a close
	 */
	inline static constexpr float ms_maxShutdownTimeSec = 5.0f;

	inline static Game* ms_currentInstance = nullptr;

	Color m_bkgColour = Color::Black;

  private:

	friend class Engine;

	#if MLGE_EDITOR
	friend class mlge::editor::Editor;
	friend class mlge::editor::GameWindow;
	#endif

	static Game* setCurrentInstance(Game* instance)
	{
		Game* previous = ms_currentInstance;
		ms_currentInstance = instance;
		return previous;
	}

	/**
	 * Called by the engine loop to tick the game using the game clock. Ends up calling tick(float deltaSeconds)
	 */
	void gameClockTick();

	std::string m_name;
	std::string m_buildInfo;
	bool m_shuttingDown = false;

	ObjectPtr<MLevel> m_level;

	GameClock m_clock;
	bool m_hasFocus = false;

	std::unique_ptr<RenderTarget> m_renderTarget;

	void onEndFrame();
	DelegateHandle m_onEndFrameHandle;

	cz::SharedQueue<std::function<void()>> m_deferedTasks;
	std::queue<std::function<void()>> m_swapDeferedTasks;
};

} // namespace mlge

std::unique_ptr<mlge::Game> createGame();


