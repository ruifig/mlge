#include "mlge/UI/UILabel.h"
#include "mlge/UI/UIScene.h"

namespace mlge
{

bool MUILabel::preConstruct()
{
	if (!Super::preConstruct())
	{
		return false;
	}

	m_textRenderer.setAlign(HAlign::Center, VAlign::Center);
	return true;
}

void MUILabel::postConstruct()
{
	Super::postConstruct();
	setStyle(m_scene->getManager().findStyle("default_label"));
}

void MUILabel::setText(std::string_view text)
{
	m_textRenderer.setText(text);
}

void MUILabel::setFont(ObjectPtr<MTTFFont> font)
{
	m_font = font;
	m_textRenderer.setFont(*m_font);
}

void MUILabel::setPtSize(int ptSize)
{
	m_textRenderer.setPtSize(ptSize);
}

void MUILabel::setAlign(HAlign halign, VAlign valign)
{
	m_textRenderer.setAlign(halign, valign);
}

void MUILabel::setStyle(const ObjectPtr<MUIStyle>& style)
{
	Super::setStyle(style);
	setFont(m_style->getFont());
}

void MUILabel::updateRenderQueue()
{
	Super::updateRenderQueue();

	if (!m_font || m_textRenderer.getText().size() == 0)
	{
		return;
	}

	RenderQueue::get().addOp(*this, RenderGroup::Overlay);
}

void MUILabel::render(RenderGroup group)
{
	Super::render(group);

	if (group == RenderGroup::Overlay && m_font && m_textRenderer.getText().size())
	{
		Rect rect = absoluteToRect(getAbsolutePosition());
		m_textRenderer.setColor(m_styleRenderer->getTextColor());
		m_textRenderer.setArea(rect).render();
	}
}

} // namespace mlge

