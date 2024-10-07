#include "mlge/Game.h"
#include "mlge/Profiler.h"
#include "mlge/Config.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Render/Renderer.h"

#include "crazygaze/core/Logging.h"
#include "timestamp.h"

using namespace std::literals::chrono_literals;

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
}

Game::~Game()
{
	CZ_LOG(Log, "Game destroyed");
}

void Game::processEvent(SDL_Event& /*evt*/)
{
}

const std::string& Game::getBuildInfo() const
{
	return m_buildInfo;
}

bool Game::init()
{
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
	CZ_LOG(VeryVerbose, "Window {}", entered ? "Enter" : "Leave");
	Game::get().windowEnterDelegate.broadcast(entered);
}

void Game::onWindowResized(const Size& size)
{
	CZ_LOG(VeryVerbose, "Window resized to {}*{}", size.w, size.h);
	getRenderTarget().setSize(size);
	windowResizedDelegate.broadcast(size);
}

void Game::gameClockTick()
{
	MLGE_PROFILE_SCOPE(mlge_Game_gameClockTick);

	// NOTE: Even if paused, calcDeltaSeconds still needs to be called to update the wall time
	float deltaSeconds = m_clock.calcDeltaSeconds();
	if (!m_clock.isPaused())
	{
		tick(deltaSeconds);
	}
}

void Game::tick(float deltaSeconds)
{
	if (m_level)
	{
		m_level->tick(deltaSeconds);
	}

	m_ui.tick(deltaSeconds);
}

void Game::requestShutdown()
{
	m_shuttingDown = true;
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

