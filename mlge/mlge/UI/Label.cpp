#include "mlge/UI/Label.h"
#include "mlge/UI/UIScene.h"

namespace mlge
{

bool MLabel::preConstruct()
{
	m_textRenderer.setAlign(HAlign::Center, VAlign::Center);
	m_textRenderer.setColor(Color::White);
	return Super::preConstruct();
}

void MLabel::postConstruct()
{
	Super::postConstruct();
	setStyle(m_scene->getManager().findStyle("default_label"));
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

void MLabel::setStyle(const ObjectPtr<MUIStyle>& style)
{
	Super::setStyle(style);
	setFont(m_style->getFont());
}

void MLabel::updateRenderQueue()
{
	Super::updateRenderQueue();

	if (!m_font || m_textRenderer.getText().size() == 0)
	{
		return;
	}

	RenderQueue::get().addOp(*this, RenderGroup::Overlay);
}

void MLabel::render(RenderGroup group)
{
	Super::render(group);

	if (group == RenderGroup::Overlay && m_font && m_textRenderer.getText().size())
	{
		Rect rect = absoluteToRect(getAbsolutePosition());
		m_textRenderer.setArea(rect).render();
	}
}

} // namespace mlge

