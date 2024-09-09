#include "Stats.h"
#include "ScratchpadGame.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Render/Renderer.h"

void FPSCalculator::tick(float deltaSeconds)
{
	numTicks++;
	std::chrono::microseconds deltaMicroseconds(int32_t(deltaSeconds * 1000 * 1000));

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

void FPSCalculator::calculateVariance()
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

void AStats::destruct()
{
	Super::destruct();
}

bool AStats::defaultConstruct()
{
	m_statsComp = addNewComponent<MTextRenderComponent>().get();
	m_font->loadASCIIGlyphs(10);
	m_statsComp->setFont(m_font);
	m_statsComp->setColor(Color::Pink);
	m_statsComp->setPtSize(12);
	m_statsComp->setRelativePosition({0, 0});
	m_statsComp->setAlignment(HAlign::Right, VAlign::Bottom);

	m_clock = addNewComponent<MTextRenderComponent>().get();
	m_clock->setFont(m_font);
	m_clock->setColor(Color::White);
	m_clock->setPtSize(24);
	m_clock->setRelativePosition({0, 0});
	m_clock->setAlignment(HAlign::Right, VAlign::Bottom);

	m_gameBuildInfo = addNewComponent<MTextRenderComponent>().get();
	m_gameBuildInfo->setFont(m_font);
	m_gameBuildInfo->setColor(Color::Pink);
	m_gameBuildInfo->setPtSize(14);
	m_gameBuildInfo->setRelativePosition({0, 0});
	m_gameBuildInfo->setAlignment(HAlign::Right, VAlign::Bottom);
	m_gameBuildInfo->setText(getGame().getBuildInfo());

	return Super::defaultConstruct();
}

void AStats::tick(float deltaSeconds)
{
	m_fpsCalc.tick(deltaSeconds);
	if (m_fpsCalc.isValid())
	{
		m_statsComp->setText(std::format(
			"FPS: {:3.0f}, MS: {:4.1f}, Variance: {:3.2f}  ", m_fpsCalc.fps, m_fpsCalc.avgMsPerFrame, m_fpsCalc.variance));
	}

	{
		auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now());
		auto nowSecs = std::chrono::time_point_cast<std::chrono::seconds>(nowMs);
		auto ms = nowMs - nowSecs;
		std::string utcTimestamp = std::format("{:%H:%M:%S}:{:03d}", nowSecs, ms.count());

		uint32_t runningSec = static_cast<uint32_t>(Game::get().getWallTimeSecs());
		uint32_t runningMs = static_cast<uint32_t>(Game::get().getWallTimeSecs() * 1000) % 1000;

		m_clock->setText(std::format("UTC Time: {}, Frame: {}, Running time: {}s {}ms", utcTimestamp, Renderer::get().getFrameNumber(), runningSec, runningMs));
	}

	Size renderTargetSize = Game::get().getRenderTarget().getSize();
	m_clock->setRelativePosition({0, static_cast<float>(renderTargetSize.h - 20 * 4)});
	m_gameBuildInfo->setRelativePosition({0, static_cast<float>(renderTargetSize.h - 20 * 2)});

	Super::tick(deltaSeconds);
}

