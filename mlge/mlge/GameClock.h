#pragma once

#include "mlge/Common.h"

namespace mlge
{

/**
 * Clock used to represent the game passage of time.
 * This allows the game to implement slow motion or fast motion by scaling the elapsed time.
 */
class GameClock
{
  public:

	/**
	 * Starts or resumes the clock (if paused)
	 */
	void start()
	{
		reset();
		m_running = true;
	}

	void stop()
	{
		m_running = false;
	}

	bool isRunning() const
	{
		return m_running;
	}

	bool isPaused() const
	{
		return m_running && m_paused;
	}

	void pause()
	{
		m_paused = true;
	}

	void resume()
	{
		m_paused = false;
	}

	void setTimeScale(float scale)
	{
		m_scale = scale;
	}

	/**
	 * Calculates the elapsed time to use for the next game tick
	 */
	float calcDeltaSeconds()
	{
		auto currentWallTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> wallDeltaSeconds = (currentWallTime - m_lastWallTime);

		m_totalWallTimeSecs += wallDeltaSeconds.count();

		float gameDeltaSeconds;
		if (m_paused)
		{
			gameDeltaSeconds = 0.0f;
		}
		else
		{
			gameDeltaSeconds = wallDeltaSeconds.count() * m_scale;
		}

		m_totalGameTimeSecs += gameDeltaSeconds;

		m_lastWallTime = currentWallTime;

		return gameDeltaSeconds;
	}

	double getGameTimeSecs() const
	{
		return m_totalGameTimeSecs;
	}

	double getWallTimeSecs() const
	{
		return m_totalWallTimeSecs;
	}

  protected:

	void reset()
	{
		using namespace std::literals::chrono_literals;

		m_running = false;
		m_paused = false;
		m_scale = 1.0f;
		m_totalGameTimeSecs = 0.0f;
		m_totalWallTimeSecs = 0.0f;
		// First tick we just fake a 16ms delta
		m_lastWallTime = std::chrono::high_resolution_clock::now() - 16ms;
	}

	bool m_running;
	bool m_paused;
	float m_scale;
	double m_totalGameTimeSecs;
	double m_totalWallTimeSecs;
	std::chrono::high_resolution_clock::time_point m_lastWallTime;
};


} // namespace mlge
