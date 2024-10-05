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
			double variance = 0;
			float fps = 0;

			using Clock = std::chrono::high_resolution_clock;
			Clock::time_point previousTs = Clock::now();

			void tick()
			{
				Clock::time_point now = Clock::now();

				numTicks++;
				auto deltaMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - previousTs);

				tickSum -= tickList[tickIndex];			 /* subtract value falling off */
				tickSum += deltaMicroseconds;			 /* add new value */
				tickList[tickIndex] = deltaMicroseconds; /* save new value so it can be subtracted later */
				if (++tickIndex == MaxSamples)			 /* inc buffer index */
				{
					tickIndex = 0;
				}

				avgMsPerFrame = float(static_cast<double>(tickSum.count()) / (MaxSamples * 1000));
				fps = 1000.0f / avgMsPerFrame;

				calculateVariance();

				previousTs = now;
			}

			void addPoint(std::chrono::microseconds deltaMicroseconds)
			{
				numTicks++;

				tickSum -= tickList[tickIndex];			 /* subtract value falling off */
				tickSum += deltaMicroseconds;			 /* add new value */
				tickList[tickIndex] = deltaMicroseconds; /* save new value so it can be subtracted later */
				if (++tickIndex == MaxSamples)			 /* inc buffer index */
				{
					tickIndex = 0;
				}

				avgMsPerFrame = float(static_cast<double>(tickSum.count()) / (MaxSamples * 1000));
				fps = 1000.0f / avgMsPerFrame;

				calculateVariance();
			}

			// Calculate Sample Variance : https://www.calculatorsoup.com/calculators/statistics/variance-calculator.php
			void calculateVariance()
			{
				double meanMs = static_cast<double>(tickSum.count()) / (MaxSamples * 1000);

				double tmp = 0;
				for (const std::chrono::microseconds& t : tickList)
				{
					double ms = static_cast<double>(t.count()) / 1000;
					tmp += std::pow(ms - meanMs, 2);
				}

				variance = tmp / (MaxSamples - 1);
			}

			bool isValid() const
			{
				return numTicks >= MaxSamples;
			}
		};
	}


class PerformanceStats : public Singleton<PerformanceStats>, public Renderable, public RenderOperation
{
  public:

	PerformanceStats();
	~PerformanceStats();
	void tick();

	void setEnabled(bool enabled);
	void showBuildInfo(bool enabled);
	void showTime(bool enabled);

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

	inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/RobotoCondensed-Medium";
	ObjectPtr<MTTFFont> m_font;
};

} // namespace mlge
