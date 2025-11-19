#pragma once

#include "mlge/Render/RenderQueue.h"
#include "mlge/Text.h"

namespace mlge
{

	namespace details
	{
		template<int NSamples>
		struct VarianceCalculator
		{
			constexpr static inline int MaxSamples = NSamples;
			int tickIndex=0;
			// Instead of keep hold of the floats, we use fixed point calculations, to avoid accumulating errors
			std::chrono::microseconds tickSum = {};
			std::chrono::microseconds tickList[MaxSamples] = {};
			float avgMsPerFrame = 0;
			uint64_t numTicks = 0;
			double variance_ms2 = 0; // Units are ms^2
			float fps = 0;

			using Clock = std::chrono::high_resolution_clock;
			Clock::time_point previousTs = Clock::now();

			/**
			 * Use this to add points based on timing between each call.
			 *
			 * For a given instance, don't mix use with start() and end(). Use either tick() or start/end.
			 */
			void tick()
			{
				Clock::time_point now = Clock::now();
				auto deltaMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - previousTs);
				addPoint(deltaMicroseconds);
				previousTs = now;
			}

			void start()
			{
				previousTs = Clock::now();
			}

			void end()
			{
				Clock::time_point now = Clock::now();
				auto deltaMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - previousTs);
				addPoint(deltaMicroseconds);
			}

			void addPoint(std::chrono::microseconds deltaMicroseconds)
			{
				const int count = static_cast<int>(std::min<uint64_t>(numTicks + 1, MaxSamples));

				tickSum -= tickList[tickIndex];			 /* subtract value falling off */
				tickSum += deltaMicroseconds;			 /* add new value */
				tickList[tickIndex] = deltaMicroseconds; /* save new value so it can be subtracted later */
				tickIndex = (tickIndex + 1) % MaxSamples; /* inc buffer index */
				++numTicks;

				avgMsPerFrame = float(static_cast<double>(tickSum.count()) / (count * 1000));
				fps = 1000.0f / avgMsPerFrame;

				calculateVariance(count);
			}

			// Calculate Sample Variance : https://www.calculatorsoup.com/calculators/statistics/variance-calculator.php
			void calculateVariance(int count)
			{
				double meanMs = tickSum.count() / (count * 1000.0);
				double tmp = 0;
				for (int i = 0; i < count; ++i)
				{
					double ms = tickList[i].count() / 1000.0;
					tmp += (ms - meanMs) * (ms - meanMs);
				}

				variance_ms2 = tmp / (count - 1);
			}

			bool isValid() const
			{
				return numTicks >= MaxSamples;
			}
		};
	}


class PerformanceStats : public Renderable, public RenderOperation
{
  public:

	PerformanceStats();
	~PerformanceStats();
	void tick();

	void setEnabled(bool enabled);
	void showBuildInfo(bool enabled);
	void showTime(bool enabled);


	// These need to ALWAYS be used in pairs.
	void stat_Tick_Start()    { m_tickCalculator.start(); }
	void stat_Tick_End()      { m_tickCalculator.end(); }
	void stat_Draw_Start()    { m_drawCalculator.start(); }
	void stat_Draw_End()      { m_drawCalculator.end(); }
	void stat_Present_Start() { m_presentCalculator.start(); }
	void stat_Present_End()   { m_presentCalculator.end(); }

	/**
	 * Get the instance associated with the game instance currently being processed
	 */
	static PerformanceStats& get();

	struct Stats
	{
		int fps;
		float avgFrametimeMs; // frametime in ms
		float variance_ms2; // Units are ms^2
	};

	Stats getStats() const;

  private:

	// Renderable interface
	virtual void updateRenderQueue() override;

	// RenderOperation interface
	virtual void render(RenderGroup group) override;

	bool m_enabled = false;
	bool m_showBuildInfo = true;
	bool m_showTime = true;

	details::VarianceCalculator<60> m_fpsCalculator;
	details::VarianceCalculator<60> m_tickCalculator;
	details::VarianceCalculator<60> m_drawCalculator;
	details::VarianceCalculator<60> m_presentCalculator;

	inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/RobotoCondensed-Medium";
	ObjectPtr<MTTFFont> m_font;
};

} // namespace mlge
