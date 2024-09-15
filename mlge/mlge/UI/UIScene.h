#pragma once

#include "mlge/Object.h"
#include "mlge/UI/Widget.h"

namespace mlge
{

MLGE_OBJECT_START(MUIScene, MObject, "Controls a series of Widgets")
class MUIScene : public MObject
{
	MLGE_OBJECT_INTERNALS(MUIScene, MObject)

  public:

	template<typename WidgetType, typename... Args>
	ObjectPtr<WidgetType> createWidget(Args&& ... args)
	{
		static_assert(!std::is_abstract_v<WidgetType>, "Widget type is abstract");
		ObjectPtr<WidgetType> widget = mlge::createObject<WidgetType>();
		if (widget)
		{
			m_widgets.push_back(widget);
		}

		return widget;
	}

  protected:

	std::vector<ObjectPtr<MWidget>> m_widgets;
};
MLGE_OBJECT_END(MUIScene)

} // namespace mlge

