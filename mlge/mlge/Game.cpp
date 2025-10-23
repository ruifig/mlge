#include "mlge/Game.h"
#include "mlge/Profiler.h"
#include "mlge/Config.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Render/Renderer.h"
#include "mlge/UI/UIScene.h"
#include "mlge/PerformanceStats.h"

#include "crazygaze/core/Logging.h"
#include "timestamp.h"

namespace mlge
{

Game::Game(std::string_view name)
	: m_name(name)
{
	const char* buildType = "";
	if constexpr (MLGE_DEBUG)
	{
		buildType = "Debug";
	}
	else if constexpr (MLGE_DEVELOPMENT)
	{
		buildType = "Development";
	}
	else if constexpr (MLGE_RELEASE)
	{
		buildType = "Release";
	}
	else
	{
		cz::details::doDebugBreak();
	}

	m_buildInfo = std::format("{} v{}, GitHash:{}, Build type:{}, Build timestamp:{} UTC",
		m_name, "0.0.0", git_short_hash_str, buildType, build_time_str);

	CZ_LOG(Main, Log, "{}", m_buildInfo);

	ms_currentInstance = this;
}

Game::~Game()
{
	CZ_LOG(Main, Log, "Game destroyed");

	// Delete these manually, because they will try to access the game
	m_performanceStats.reset();
	m_ui.reset();
	m_renderQueue.reset();

	ms_currentInstance = nullptr;
}

const std::string& Game::getBuildInfo() const
{
	return m_buildInfo;
}

bool Game::init()
{
	m_renderQueue = std::make_unique<RenderQueue>();
	m_ui = std::make_unique<UIManager>();
	m_performanceStats = std::make_unique<PerformanceStats>();
	m_performanceStats->setEnabled(true);

	Size windowSize;
	windowSize.w = Config::get().getValueOrDefault<int>("Engine", "resx", 0);
	windowSize.h = Config::get().getValueOrDefault<int>("Engine", "resy", 0);

	if (windowSize.w == 0 || windowSize.h == 0)
	{
		windowSize.w = 1024;
		windowSize.h = 768;

		Config::get().setGameValue("Engine", "resx", windowSize.w);
		Config::get().setGameValue("Engine", "resy", windowSize.h);
		Config::get().save();
	}

	if (gIsGame)
	{
		m_renderTarget = std::make_unique<WindowRenderTarget>();
	}
	else
	{
		m_renderTarget = std::make_unique<TextureRenderTarget>();
	}
	m_renderTarget->init({windowSize.w, windowSize.h});

	m_onEndFrameHandle = Renderer::get().endFrameDelegate.bind(this, &Game::onEndFrame);

	m_clock.start();

	return true;
}

void Game::onEndFrame()
{
	Renderer::get().setTarget(m_renderTarget.get());
	Renderer::get().clearTarget(m_bkgColour);
}

void Game::onWindowEnter(bool entered)
{
	CZ_LOG(Main, VeryVerbose, "Window {}", entered ? "Enter" : "Leave");
	Game::get().windowEnterDelegate.broadcast(entered);
}

void Game::onWindowResized(const Size& size)
{
	CZ_LOG(Main, VeryVerbose, "Window resized to {}*{}", size.w, size.h);
	getRenderTarget().setSize(size);
	windowResizedDelegate.broadcast(size);
}

void Game::onWindowFocus(bool focus)
{
	CZ_LOG(Main, VeryVerbose, "Window {} focus", focus ? "gained" : "lost");
	m_hasFocus = focus;
	windowFocus.broadcast(focus);
}

void Game::onMouseMotion(const MouseMotionEvent& evt)
{
	//CZ_LOG(Main, VeryVerbose, "MouseMotionEvent: Pos=({},{}) , Rel=({},{})", evt.pos.x, evt.pos.y, evt.rel.x, evt.rel.y);
	mouseMotionDelegate.broadcast(evt);
}

void Game::gameClockTick()
{
	MLGE_PROFILE_SCOPE(mlge_Game_gameClockTick);

	// NOTE: Even if paused, calcDeltaSeconds still needs to be called to update the wall time
	float deltaSeconds = m_clock.calcDeltaSeconds();
	if (!m_clock.isPaused())
	{
		if (m_deferedTasks.popAll(m_swapDeferedTasks))
		{
			while(m_swapDeferedTasks.size())
			{
				// Note the double ()(). Intentional.
				m_swapDeferedTasks.front()();
				m_swapDeferedTasks.pop();
			}
		}

		tick(deltaSeconds);
	}
}

void Game::tick(float deltaSeconds)
{
	if (m_level)
	{
		m_level->tick(deltaSeconds);
	}

	m_ui->tick(deltaSeconds);
}

void Game::requestExpectedShutdown()
{
	m_shuttingDown = true;
}

void Game::requestShutdownWithError()
{
	m_shuttingDown = false;
}

float Game::startShutdown()
{
	m_clock.setTimeScale(1.0f);
	m_clock.resume();
	return 0.0f;
}

bool Game::isShutdownFinished()
{
	return true;
}

void Game::shutdown()
{
}

mlge::MLevel& Game::getLevel()
{
	if (!m_level)
	{
		m_level = createObject<MLevel>();
	}

	return *m_level;
}

} // namespace mlge

