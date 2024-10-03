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

	enum class State
	{
		Disabled,
		Enabled,
		Active
	};

	MWidget& getRootWidget()
	{
		return *m_rootWidget;
	}

	virtual void tick(float deltaSeconds);

  protected:

	virtual bool preConstruct() override;
	virtual void destruct() override;

	ObjectPtr<MWidget> m_rootWidget;

	friend MWidget;

	void addWidget(MWidget& widget);
	void removeWidget(MWidget& widget);
	void onUIEvent(UIEvent& evt);

	// All the widgets in this scene. This is non-owning relationship.
	// - A widget lifetime is controlled by its parent
	// - A widget adds and removes itself from this vector
	std::vector<MWidget*> m_widgets;

	State m_state = State::Disabled;
};
MLGE_OBJECT_END(MUIScene)

} // namespace mlge

