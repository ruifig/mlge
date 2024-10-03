#pragma once

#include "mlge/Game.h"
#include "mlge/Actor.h"
#include "mlge/Text.h"

using namespace mlge;

struct FPSCalculator
{
	constexpr static inline int MaxSamples = 100;
	int tickIndex=0;
	// Instead of keep hold of the floats, we use fixed point calculations, to avoid accumulating errors
	std::chrono::microseconds tickSum = {};
	std::chrono::microseconds tickList[MaxSamples] = {};
	float fps = 0;
	float avgMsPerFrame = 0;
	uint64_t numTicks = 0;

	void tick(float deltaSeconds);
	double variance = 0;

	// Calculate Sample Variance : https://www.calculatorsoup.com/calculators/statistics/variance-calculator.php
	void calculateVariance();

	bool isValid() const
	{
		return numTicks >= MaxSamples;
	}
};

MLGE_OBJECT_START(AStats, AActor, "Stats Actor")
class AStats : public AActor
{
	MLGE_OBJECT_INTERNALS(AStats, AActor)

  public:

	virtual void destruct() override;
	virtual bool preConstruct() override;
	virtual void tick(float deltaSeconds) override;

  protected:

	inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/RobotoCondensed-Medium";
	ResourceRef<MTTFFont> m_font = ms_fontRef;
	MTextRenderComponent* m_statsComp = nullptr;
	MTextRenderComponent* m_clock = nullptr;
	MTextRenderComponent* m_gameBuildInfo = nullptr;
	FPSCalculator m_fpsCalc;
};
MLGE_OBJECT_END(AStats)

