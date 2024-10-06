#include "UIWidget.h"
#include "crazygaze/core/Logging.h"
#include "crazygaze/core/Algorithm.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Game.h"
#include "mlge/UI/UIScene.h"
#include "mlge/Render/DebugUtils.h"

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

bool MUIWidget::preConstruct()
{
	return true;
}

bool MUIWidget::construct(MUIScene& scene)
{
	m_scene = &scene;
	return true;
}

bool MUIWidget::construct(MUIWidget& parent)
{
	m_parent = &parent;
	m_scene = parent.m_scene; 
	return true;
}

void MUIWidget::postConstruct()
{
	Super::postConstruct();
	m_scene->addWidget(*this);
	setStyle(m_scene->getManager().findStyle("default"));
}

void MUIWidget::destruct()
{
	m_scene->removeWidget(*this);

	Super::destruct();
}

const WidgetRect& MUIWidget::getAbsolutePosition() const
{
	updateAbsolutePosition();
	return m_screenPos;
}

void MUIWidget::setPosition(const WidgetRect& rect)
{
	m_pos = rect;
	m_posChanged = true;
}

void MUIWidget::setStyle(const ObjectPtr<MUIStyle>& style)
{
	if (style)
	{
		m_style = style;
		m_styleRenderer = m_style->createRenderer();
		setEnabled(true);
	}

}

void MUIWidget::onWindowResized()
{
	m_posChanged = true;
	for (auto&& child : m_children)
	{
		child->onWindowResized();
	}
}

void MUIWidget::setEnabled(bool enabled)
{
	m_enabled = enabled;
	propagateEnabled();
}

void MUIWidget::propagateEnabled()
{
	if (m_parent)
	{
		m_parentEnabled = m_parent->isEnabled();
	}
	else
	{
		m_parentEnabled = m_scene->getState() == MUIScene::State::Disabled ? false : true;
	}


	CZ_LOG(Verbose, "{}:{}: {}", m_objectName, __FUNCTION__, isEnabled());
	m_styleRenderer->setEnabled(isEnabled());

	for (auto child : m_children)
	{
		child->propagateEnabled();
	}
}

void MUIWidget::onMouseEnter()
{
	CZ_LOG(Verbose, "{}:{}", m_objectName, __FUNCTION__);
	CZ_CHECK(m_mouseHover == false);
	m_mouseHover = true;
	m_styleRenderer->setHover(true);
}

void MUIWidget::onMouseLeave()
{
	CZ_LOG(Verbose, "{}:{}", m_objectName, __FUNCTION__);
	CZ_CHECK(m_mouseHover);

	m_mouseHover = false;
	m_styleRenderer->setHover(false);
}

void MUIWidget::updateAbsolutePosition() const
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

bool MUIWidget::containsPoint(const Point& pt)
{
	updateAbsolutePosition();

	return
		(pt.x >= static_cast<int>(m_screenPos.tl.x.val)) && (pt.x < static_cast<int>(m_screenPos.br.x.val)) &&
		(pt.y >= static_cast<int>(m_screenPos.tl.y.val)) && (pt.y < static_cast<int>(m_screenPos.br.y.val));
}

void MUIWidget::updateRenderQueue()
{
	RenderQueue::get().addOp(*this, RenderGroup::Overlay);

	#if MLGE_DEBUG
		RenderQueue::get().addOp(*this, RenderGroup::OverlayDebug);
	#endif
}

void MUIWidget::render(RenderGroup group)
{
	Rect rect = absoluteToRect(getAbsolutePosition());

	if (group == RenderGroup::Overlay)
	{
		m_styleRenderer->render(rect);
	}
	#if MLGE_DEBUG
	else if (group == RenderGroup::OverlayDebug)
	{
		drawDebugOverlayRect(rect);
	}
	#endif
}

void MUIWidget::onUIInternalEvent(UIInternalEvent& evt)
{
	//if (!containsPoint(evt.pos))
	//{
	//	return;
	//}

	CZ_LOG(Verbose, "{}:{}", m_objectName, __FUNCTION__);
}

void MUIWidget::tick(float deltaSeconds)
{
	for(auto&& child : m_children)
	{
		child->tick(deltaSeconds);
	}
}

void MUIWidget::processMouseCursor(const Point& pos, std::vector<MUIWidget*>& eventStack)
{
	if (containsPoint(pos))
	{
		if (isEnabled())
		{
			eventStack.push_back(this);
		}

		if (!m_mouseHover)
		{
			onMouseEnter();
		}
	}
	else
	{
		if (m_mouseHover)
		{
			onMouseLeave();
		}
	}

	for(const ObjectPtr<MUIWidget>& child : m_children)
	{
		child->processMouseCursor(pos, eventStack);
	}
}

} // namespace mlge

