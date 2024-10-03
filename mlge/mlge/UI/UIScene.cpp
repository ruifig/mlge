#include "mlge/UI/UIScene.h"
#include "crazygaze/core/Algorithm.h"

namespace mlge
{

bool MUIScene::preConstruct()
{
	m_rootWidget = createObject<MWidget>(*this);
	m_rootWidget->setPosition(WidgetRect(UIUnitType::Percentage, 0, 0, 1, 1));
	return true;
}

void MUIScene::destruct()
{
	m_rootWidget = nullptr;
}

void MUIScene::addWidget(MWidget& widget)
{
	m_widgets.push_back(&widget);
	RenderQueue::get().addRenderable(widget);
}

void MUIScene::removeWidget(MWidget& widget)
{
	cz::remove(m_widgets, &widget);
	RenderQueue::get().removeRenderable(widget);
}

void MUIScene::onUIEvent(UIEvent& evt)
{
	m_rootWidget->onUIEvent(evt);
}

void MUIScene::tick(float deltaSeconds)
{
	m_rootWidget->tick(deltaSeconds);
}

} // namespace mlge

