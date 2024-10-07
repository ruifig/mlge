#include "mlge/UI/UIButton.h"
#include "mlge/UI/UIScene.h"

namespace mlge
{

bool MUIButton::preConstruct()
{
	if (!Super::preConstruct())
	{
		return false;
	}

	m_textRenderer.setAlign(HAlign::Center, VAlign::Center);
	return true;
}

void MUIButton::postConstruct()
{
	Super::postConstruct();
	setStyle(m_scene->getManager().findStyle("default_button"));
}

void MUIButton::setText(std::string_view text)
{
	m_textRenderer.setText(text);
}

void MUIButton::setFont(ObjectPtr<MTTFFont> font)
{
	m_font = font;
	m_textRenderer.setFont(*m_font);
}

void MUIButton::setPtSize(int ptSize)
{
	m_textRenderer.setPtSize(ptSize);
}

void MUIButton::setAlign(HAlign halign, VAlign valign)
{
	m_textRenderer.setAlign(halign, valign);
}

void MUIButton::setStyle(const ObjectPtr<MUIStyle>& style)
{
	Super::setStyle(style);
	setFont(m_style->getFont());
}

void MUIButton::setEventID(std::string_view id)
{
	m_eventId.id = id;
	m_eventId.hash = Hash::fnv_64a_str(m_eventId.id.c_str());
}

void MUIButton::updateRenderQueue()
{
	Super::updateRenderQueue();

	if (!m_font || m_textRenderer.getText().size() == 0)
	{
		return;
	}

	RenderQueue::get().addOp(*this, RenderGroup::Overlay);
}

void MUIButton::render(RenderGroup group)
{
	Super::render(group);

	if (group == RenderGroup::Overlay && m_font && m_textRenderer.getText().size())
	{
		Rect rect = absoluteToRect(getAbsolutePosition());
		m_textRenderer.setColor(m_styleRenderer->getTextColor());
		m_textRenderer.setArea(rect).render();
	}
}

void MUIButton::onMouseEnter()
{
	Super::onMouseEnter();
}

void MUIButton::onMouseLeave()
{
	Super::onMouseLeave();
}

bool MUIButton::onClicked(const Point& pos)
{
	Super::onClicked(pos);

	if (m_eventId.id.size())
	{
		UIEvent e;
		e.name = m_eventId.id;
		e.hash = m_eventId.hash;
		e.source = this;
		CZ_LOG(Verbose, "{}: Clicked", m_objectName);
		m_scene->getManager().uiEventDelegate.broadcast(e);
	}
	else
	{
		CZ_LOG(Warning, "{}: Clicked but no event ID set", m_objectName);
	}

	return true;
}


} // namespace mlge

