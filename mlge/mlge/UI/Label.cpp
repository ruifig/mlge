#include "mlge/UI/Label.h"
#include "mlge/Render/DebugUtils.h"

namespace mlge
{

bool MLabel::preConstruct()
{
	m_textRenderer.setAlign(HAlign::Center, VAlign::Center);
	m_textRenderer.setColor(Color::White);
	setFont(MWidget::defaultFontRef.getResource());
	return Super::preConstruct();
}

void MLabel::setText(std::string_view text)
{
	m_textRenderer.setText(text);
}

void MLabel::setFont(ObjectPtr<MTTFFont> font)
{
	m_font = font;
	m_textRenderer.setFont(*m_font);
}

void MLabel::setPtSize(int ptSize)
{
	m_textRenderer.setPtSize(ptSize);
}

void MLabel::setAlign(HAlign halign, VAlign valign)
{
	m_textRenderer.setAlign(halign, valign);
}

void MLabel::updateRenderQueue()
{
	Super::updateRenderQueue();

	if (!m_font || m_textRenderer.getText().size() == 0)
	{
		return;
	}

	RenderQueue::get().addOp(*this, RenderGroup::Overlay);

	#if MLGE_DEBUG
		RenderQueue::get().addOp(*this, RenderGroup::OverlayDebug);
	#endif
}

void MLabel::render(RenderGroup group)
{
	Rect rect = absoluteToRect(getAbsolutePosition());

	if (group == RenderGroup::Overlay)
	{
		m_textRenderer.setArea(rect).render();
	}
	#if MLGE_DEBUG
	else if (group == RenderGroup::OverlayDebug)
	{
		drawDebugOverlayRect(rect);
	}
	#endif
}

} // namespace mlge

