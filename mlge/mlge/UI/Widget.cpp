#include "Widget.h"
#include "crazygaze/core/Logging.h"
#include "crazygaze/core/Algorithm.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Game.h"
#include "mlge/UI/UIScene.h"

namespace mlge
{


WidgetUnit toAbsolute(const WidgetUnit& u, float from, float to)
{
	float res;
	if (u.type == UIUnitType::Absolute)
	{
		res = u.val;
	}
	else if (u.type == UIUnitType::Relative)
	{
		res = from + u.val;
	}
	else
	{
		res = from + ((to - from) * u.val);
	}

	return {UIUnitType::Absolute, res};
}

WidgetPoint toAbsolute(const WidgetPoint& p, const WidgetRect& parentAbsolute)
{
	CZ_CHECK(parentAbsolute.isAbsolute());

	return {
		toAbsolute(p.x, parentAbsolute.tl.x.val, parentAbsolute.br.x.val),
		toAbsolute(p.y, parentAbsolute.tl.y.val, parentAbsolute.br.y.val)
	};
}

WidgetRect toAbsolute(const WidgetRect& r, const WidgetRect& parentAbsolute)
{
	CZ_CHECK(parentAbsolute.isAbsolute());

	return {
		toAbsolute(r.tl, parentAbsolute),
		toAbsolute(r.br, parentAbsolute)
	};
}

Rect absoluteToRect(const WidgetRect& r)
{
	CZ_CHECK(r.isAbsolute());

	return Rect(
		static_cast<int>(r.tl.x.val), static_cast<int>(r.tl.y.val),
		static_cast<int>(r.br.x.val - r.tl.x.val), static_cast<int>(r.br.y.val - r.tl.y.val));
}

//////////////////////////////////////////////////////////////////////////
// Widget
//////////////////////////////////////////////////////////////////////////

bool MWidget::preConstruct()
{
	return true;
}

bool MWidget::construct(MUIScene& scene)
{
	m_scene = &scene;
	return true;
}

bool MWidget::construct(MWidget& parent)
{
	m_parent = &parent;
	m_scene = parent.m_scene; 
	return true;
}

void MWidget::postConstruct()
{
	Super::postConstruct();
	m_scene->addWidget(*this);
}

void MWidget::destruct()
{
	m_scene->removeWidget(*this);
	//m_children.clear();

	//if (m_parent)
	//{
	//	cz::remove_if(m_parent->m_children, [this](const ObjectPtr<MWidget>& ele)
	//	{
	//		return ele.get() == this;
	//	});
	//}

	Super::destruct();
}

const WidgetRect& MWidget::getAbsolutePosition() const
{
	updateAbsolutePosition();
	return m_screenPos;
}

void MWidget::setPosition(const WidgetRect& rect)
{
	m_pos = rect;
	m_posChanged = true;
}

void MWidget::updateAbsolutePosition() const
{
	if (!m_posChanged)
	{
		return;
	}

	if (m_parent)
	{
		m_screenPos = toAbsolute(m_pos, m_parent->getAbsolutePosition());
	}
	else
	{
		Size screenSize = Game::get().getRenderTarget().getSize();
		m_screenPos = toAbsolute(
			m_pos, WidgetRect(UIUnitType::Absolute, 0, 0, static_cast<float>(screenSize.w), static_cast<float>(screenSize.h)));
	}

	m_posChanged = false;
}

bool MWidget::containsPoint(const Point& pt)
{
	updateAbsolutePosition();

	return
		(pt.x >= static_cast<int>(m_screenPos.tl.x.val)) && (pt.x < static_cast<int>(m_screenPos.br.x.val)) &&
		(pt.y >= static_cast<int>(m_screenPos.tl.y.val)) && (pt.y < static_cast<int>(m_screenPos.br.y.val));
}

void MWidget::updateRenderQueue()
{
}

void MWidget::onUIEvent(UIEvent& evt)
{
	if (!containsPoint(evt.pos))
	{
		return;
	}

	for(auto&& widget : m_children)
	{
		widget->onUIEvent(evt);
	}
}

void MWidget::tick(float deltaSeconds)
{
	for(auto&& child : m_children)
	{
		child->tick(deltaSeconds);
	}
}

} // namespace mlge

