#pragma once

#include "mlge/Common.h"
#include "mlge/Object.h"
#include "mlge/Render/RenderQueue.h"
#include "mlge/UI/UIEvent.h"
#include "mlge/UI/UIStyle.h"
#include "mlge/Text.h"

namespace mlge
{


class MUIScene;

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
	// TopLeft point (inclusive)
	WidgetPoint tl;

	// BottomRight point (exclusive)
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

/**
 * Given a WidgetUnit that is expressed in absolute units, it returns a Rect that can be used by the rendering API
 */
Rect absoluteToRect(const WidgetRect& r);

MLGE_OBJECT_START(MWidget, MObject, "Base class for all UI widgets")
class MWidget : public MObject, public Renderable, public RenderOperation
{
	MLGE_OBJECT_INTERNALS(MWidget, MObject)

  public:

	bool construct(MUIScene& scene); // Only used by a root widget
	bool construct(MWidget& parent);

	const WidgetRect& getAbsolutePosition() const;
	virtual void setPosition(const WidgetRect& rect);

	virtual void setStyle(const ObjectPtr<MUIStyle>& style);

	/**
	 * Creates a child widget.
	 * The lifetime of the child widget is tied to the parent.
	 * Returns a pointer to the child widget, or nullptr if creation failed.
	 */
	template<typename WidgetType, typename... Args>
	WeakObjectPtr<WidgetType> createChild(Args&& ... args)
	{
		static_assert(!std::is_abstract_v<WidgetType>, "Widget type is abstract");
		ObjectPtr<WidgetType> w = createObject<WidgetType>(*this, std::forward<Args>(args)...);
		if (w)
		{
			m_children.push_back(w);
		}

		return w;
	}

	MUIScene& getScene()
	{
		return *m_scene;
	}

  protected:

	virtual bool preConstruct() override;
	virtual void destruct() override;
	virtual void postConstruct() override;

	virtual void tick(float deltaSeconds);

	void updateAbsolutePosition() const;
	bool containsPoint(const Point& pt);

	friend MUIScene;

	/**
	 * Processes an UI event.
	 * 
	 * If the widget takes care of the event, it should set `consumed` to true.
	 */
	virtual void onUIEvent(UIEvent& evt);

	//
	// Renderable interface
	//
	virtual void updateRenderQueue() override;

	//
	// RenderOperation interface
	//
	virtual void render(RenderGroup group) override; 


	// UIScene this widget belongs to
	MUIScene* m_scene = nullptr;
	// Parent widget, if inside another widget
	MWidget* m_parent = nullptr;

	std::vector<ObjectPtr<MWidget>> m_children;

	WidgetRect m_pos = {};

	mutable bool m_posChanged = false;
	mutable WidgetRect m_screenPos;

	ObjectPtr<MUIStyle> m_style;
	ObjectPtr<MUIStyleRenderer> m_styleRenderer;

#if MLGE_DEBUG
	RenderGroup m_debugRenderGroup = RenderGroup::OverlayDebug;
#endif

};
MLGE_OBJECT_END(MWidget)


} // namespace mlge

