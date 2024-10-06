#pragma once

#include "mlge/Common.h"
#include "mlge/Object.h"
#include "mlge/Math.h"
#include "mlge/Text.h"

namespace mlge
{

MLGE_OBJECT_START(MUIStyleRenderer, MObject, "Renders a widget with a specific style")
class MUIStyleRenderer : public MObject
{
	MLGE_OBJECT_INTERNALS_ABSTRACT(MUIStyleRenderer, MObject)

  public:

	virtual ~MUIStyleRenderer() = default;

	virtual void setEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	virtual void setHover(bool hover)
	{
		m_hover = hover;
	}

	virtual void setPressed(bool pressed)
	{
		m_pressed = pressed;
	}

	virtual void render(const Rect& rect) = 0;

	/**
	 * Returns what color should be used for text, taking into account the current state
	 */
	virtual Color getTextColor()
	{
		return Color::White;
	}

  protected:

		//! The widget is enabled
		bool m_enabled : 1 = true;

		//! The mouse cursor is over the widget
		bool m_hover : 1 = false;

		//! Currently being pressed
		bool m_pressed : 1 = false;
};
MLGE_OBJECT_END(MUIStyleRenderer)

MLGE_OBJECT_START(MUIStyle, MObject, "Provides rendering for UI")
class MUIStyle : public MObject
{
	MLGE_OBJECT_INTERNALS_ABSTRACT(MUIStyle, MObject)

  public:

	/**
	 * Creates a renderer for the style.
	 *
	 * A renderer encapsulates the styling and also some widget state, and therefore should not be shared between widgets.
	 * Every widget needs to have a renderer, and cannot be shared with other instances.
	 * @warning The MUIStyle that created a MUIStyleRenderer must outlive the renderer.
	 */
	virtual ObjectPtr<MUIStyleRenderer> createRenderer() = 0;

	/**
	 * Returns the default font for this style
	 */
	ObjectPtr<MTTFFont> getFont();

  private:

	inline static StaticResourceRef<MTTFFont> ms_defaultFontRef = "fonts/RobotoCondensed-Medium";
	ObjectPtr<MTTFFont> m_font;
};
MLGE_OBJECT_END(MUIStyle)

//////////////////////////////////////////////////////////////////////////
// Empty style (doesn't render anything)
//////////////////////////////////////////////////////////////////////////

class MUIStyleEmpty;

MLGE_OBJECT_START(MUIStyleRendererEmpty, MUIStyleRendererEmpty, "Widget renderer that doesn't do anything")
class MUIStyleRendererEmpty : public MUIStyleRenderer
{
	MLGE_OBJECT_INTERNALS(MUIStyleRendererEmpty, MUIStyleRenderer)

  public:

	bool construct(MUIStyleEmpty& outer)
	{
		m_outer = &outer;
		return true;
	}
	virtual void render(const Rect& /*rect*/) override
	{
	}

  private:

	MUIStyleEmpty* m_outer = nullptr;
};
MLGE_OBJECT_END(MUIStyleRendererEmpty)

MLGE_OBJECT_START(MUIStyleEmpty, MUIStyle, "Widget style that doesn't do anything")
class MUIStyleEmpty : public MUIStyle
{
	MLGE_OBJECT_INTERNALS(MUIStyleEmpty, MUIStyle)

  public:

	virtual ObjectPtr<MUIStyleRenderer> createRenderer() override
	{
		return createObject<MUIStyleRendererEmpty>(*this);
	}

  protected:
};
MLGE_OBJECT_END(MUIStyleEmpty)

//////////////////////////////////////////////////////////////////////////
// Flat colour rendering
//////////////////////////////////////////////////////////////////////////

class MUIStyleFlat;

MLGE_OBJECT_START(MUIStyleRendererFlat, MUIStyleRenderer, "Renders a widget with a flat color")
class MUIStyleRendererFlat : public MUIStyleRenderer
{
	MLGE_OBJECT_INTERNALS(MUIStyleRendererFlat, MUIStyleRenderer)

  public:

	bool construct(MUIStyleFlat& outer);

	virtual Color getTextColor() override;

	virtual void render(const Rect& rect) override;

  private:

	MUIStyleFlat* m_outer = nullptr;
};
MLGE_OBJECT_END(MUIStyleRendererFlat)

MLGE_OBJECT_START(MUIStyleFlat, MUIStyle, "Provides rendering for UI, using a flat solid color")
class MUIStyleFlat : public MUIStyle
{
	MLGE_OBJECT_INTERNALS(MUIStyleFlat, MUIStyle)

  public:

	virtual ObjectPtr<MUIStyleRenderer> createRenderer() override;

  protected:

	friend MUIStyleRendererFlat;

	static constexpr uint8_t ms_disabledAlpha = 64; 

	Color m_disabledBkgColor = Color(56 , 0, 44, ms_disabledAlpha);
	Color m_enabledBkgColor = Color(56 , 0, 44, 255);
	Color m_hoverBkgColor = Color(86 , 106, 137, 255);
	Color m_pressedBkgColor = Color(101 , 124, 160, 255);

	Color m_textColor = Color::White;

	/**
	 * How thick (in pixels) the border should be.
	 * If 0, then no border is drawn
	 */
	int m_borderThickness = 2;
	Color m_borderColor = Color::Black;
};
MLGE_OBJECT_END(MUIStyleFlat)

} // namespace mlge


