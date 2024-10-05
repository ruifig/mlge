#include "PerformanceStats.h"
#include "mlge/Game.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

PerformanceStats::PerformanceStats()
{
	m_font = ms_fontRef.getResource();
}

PerformanceStats::~PerformanceStats()
{
	RenderQueue::get().removeRenderable(*this);
}

void PerformanceStats::setEnabled(bool enabled)
{
	if (m_enabled == enabled)
	{
		return;
	}

	m_enabled = enabled;

	if (enabled)
	{
		RenderQueue::get().addRenderable(*this);
	}
	else
	{
		RenderQueue::get().removeRenderable(*this);
	}
}

void PerformanceStats::tick()
{
	if (!m_enabled || Game::tryGet() == nullptr)
	{
		return;
	}

	m_fpsCalculator.tick();
}

void PerformanceStats::updateRenderQueue()
{
	RenderQueue::get().addOp(*this, RenderGroup::Stats);
}

void PerformanceStats::render(RenderGroup /*group*/)
{
	if (Game::tryGet() == nullptr)
	{
		return;
	}

	Size renderTargetSize = Game::get().getRenderTarget().getSize();
	TextRenderer<false> textRenderer;
	textRenderer.setFont(*m_font);
	textRenderer.setPtSize(12);
	textRenderer.setColor(Color::Pink);
	textRenderer.setAlign(HAlign::Left, VAlign::Bottom);
	int x = 5;
	Rect rect(x, 0, renderTargetSize.w - x, textRenderer.getFontHeight());

	textRenderer.setArea(rect);
	textRenderer.render(std::format(
		"FPS: {:3.0f}, MS: {:4.1f}, Variance: {:3.2f}",
		m_fpsCalculator.fps,
		m_fpsCalculator.avgMsPerFrame,
		m_fpsCalculator.variance));

	int buildInfoHeight = m_showBuildInfo ? textRenderer.getFontHeight() : 0;

	if (m_showBuildInfo)
	{
		rect.move({x, renderTargetSize.h - buildInfoHeight});
		textRenderer.setArea(rect);
		textRenderer.render(Game::get().getBuildInfo());
	}

	if (m_showTime)
	{
		textRenderer.setColor(Color::White);
		textRenderer.setPtSize(24);
		rect.move({x, renderTargetSize.h - buildInfoHeight - textRenderer.getFontHeight()});
		rect.h = textRenderer.getFontHeight();
		textRenderer.setArea(rect);
		
		auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now());
		auto nowSecs = std::chrono::time_point_cast<std::chrono::seconds>(nowMs);
		auto ms = nowMs - nowSecs;
		std::string utcTimestamp = std::format("{:%H:%M:%S}:{:03d}", nowSecs, ms.count());

		uint32_t runningSec = static_cast<uint32_t>(Game::get().getWallTimeSecs());
		uint32_t runningMs = static_cast<uint32_t>(Game::get().getWallTimeSecs() * 1000) % 1000;

		textRenderer.render(std::format("UTC Time: {}, Frame: {}, Running time: {}s {}ms", utcTimestamp, Renderer::get().getFrameNumber(), runningSec, runningMs));
	}
}

} // namespace mlge

