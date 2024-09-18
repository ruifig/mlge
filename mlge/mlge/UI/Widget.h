#pragma once

#include "mlge/Common.h"
#include "mlge/Object.h"
#include "mlge/Render/RenderQueue.h"

namespace mlge
{


enum class UIUnitType
{
	// Absolute pixel values (in screen space)
	Absolute,
	// Pixel values, relative to parent widget
	Relative, 
	// Percentage relative to parent widget
	Percentage
};

struct WidgetUnit
{
	UIUnitType type = UIUnitType::Absolute;
	float val = 0;
	WidgetUnit() = default;
	WidgetUnit(UIUnitType type, float val)
		: type(type)
		, val(val)
	{
	}

	bool isAbsolute() const
	{
		return type == UIUnitType::Absolute;
	}
};

struct WidgetPoint
{
	WidgetUnit x;
	WidgetUnit y;

	WidgetPoint() = default;
	WidgetPoint(UIUnitType type, float x, float y)
		: x(type, x)
		, y(type, y)
	{
	}
	WidgetPoint(const WidgetUnit& x, const WidgetUnit& y)
		: x(x)
		, y(y)
	{
	}

	bool isAbsolute() const
	{
		return x.isAbsolute() && y.isAbsolute();
	}
};

struct WidgetRect
{
	// TopLeft point
	WidgetPoint tl;
	// BottomRight point
	WidgetPoint br;

	WidgetRect() = default;
	WidgetRect(UIUnitType type, float left, float top, float right, float bottom)
		: tl(type, left, top)
		, br(type, right, bottom)
	{
	}
	WidgetRect(const WidgetPoint& tl, const WidgetPoint& br)
		: tl(tl)
		, br(br)
	{
	}

	bool isAbsolute() const
	{
		return tl.isAbsolute() && br.isAbsolute();
	}
};

/**
 * Returns a WidgetUnit converted to screen space.
 * The parameters are the from->to value to as relative position. E.g, if converting a widget's position to absolute position,
 * the conversion needs to know the parent's absolute values so it can calculate it's own position (if it is a relative
 * position)
 */
WidgetUnit toAbsolute(const WidgetUnit& u, float from, float to);

/**
 * Returns a point converted to screen space.
 * @param parentAbsolute The rectangle of the parent in screen space.
 */
WidgetPoint toAbsolute(const WidgetPoint& p, const WidgetRect& parentAbsolute);

/**
 * Returns a rectangle converted to screen space.
 * @param parentAbsolute The rectangle of the parent in screen space.
 */
WidgetRect toAbsolute(const WidgetRect& r, const WidgetRect& parentAbsolute);


MLGE_OBJECT_START(MWidget, MObject, "Base class for all UI widgets")
class MWidget : public MObject, public Renderable
{
	MLGE_OBJECT_INTERNALS(MWidget, MObject)

  public:

	const WidgetRect& getAbsolutePos() const;

  protected:

	//
	// Renderable interface
	//
	virtual void updateRenderQueue() override;

	void updateAbsolutePos() const;

	MWidget* m_parent = nullptr;
	std::vector<ObjectPtr<MWidget>> m_children;
	WidgetRect m_pos = {};

	mutable bool m_posChanged = false;
	mutable WidgetRect m_screenPos;

#if MLGE_DEBUG
	RenderGroup m_debugRenderGroup = RenderGroup::OverlayDebug;
#endif

};
MLGE_OBJECT_END(MWidget)


} // namespace mlge

