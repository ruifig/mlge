#include "Widget.h"
#include "crazygaze/core/Logging.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Game.h"

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

//////////////////////////////////////////////////////////////////////////
// Widget
//////////////////////////////////////////////////////////////////////////

const mlge::WidgetRect& MWidget::getAbsolutePos() const
{
	updateAbsolutePos();
	return m_pos;
}

void MWidget::updateAbsolutePos() const
{
	if (!m_posChanged)
	{
		return;
	}

	if (m_parent)
	{
		m_screenPos = toAbsolute(m_pos, m_parent->getAbsolutePos());
	}
	else
	{
		Size screenSize = Game::get().getRenderTarget().getSize();
		m_screenPos = toAbsolute(
			m_pos, WidgetRect(UIUnitType::Absolute, 0, 0, static_cast<float>(screenSize.w), static_cast<float>(screenSize.h)));
	}

	m_posChanged = false;
}

void MWidget::updateRenderQueue()
{

}

} // namespace mlge

