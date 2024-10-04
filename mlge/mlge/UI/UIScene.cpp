#include "mlge/UI/UIScene.h"
#include "crazygaze/core/Algorithm.h"

namespace mlge
{

bool MUIScene::preConstruct()
{
	return true;
}

bool MUIScene::construct(UIManager& outer, std::string_view name)
{
	m_outer = &outer;
	m_name = name;

	m_rootWidget = createObject<MWidget>(*this);
	m_rootWidget->setPosition(WidgetRect(UIUnitType::Percentage, 0, 0, 1, 1));
	m_rootWidget->setStyle(m_outer->findStyle("empty"));

	return true;
}


void MUIScene::destruct()
{
	m_rootWidget = nullptr;
}

void MUIScene::addWidget(MWidget& widget)
{
	m_widgets.push_back(&widget);

	if (m_state >= State::Enabled)
	{
		RenderQueue::get().addRenderable(widget);
	}
}

void MUIScene::removeWidget(MWidget& widget)
{
	cz::remove(m_widgets, &widget);

	if (m_state >= State::Enabled)
	{
		RenderQueue::get().removeRenderable(widget);
	}
}

void MUIScene::onUIEvent(UIEvent& evt)
{
	m_rootWidget->onUIEvent(evt);
}

void MUIScene::tick(float deltaSeconds)
{
	m_rootWidget->tick(deltaSeconds);
}

void MUIScene::onEnable()
{
	for(MWidget* w : m_widgets)
	{
		RenderQueue::get().addRenderable(*w);
	}
}

void MUIScene::onActivate()
{
}

void MUIScene::onDeactivate()
{
}

void MUIScene::onDisable()
{
	for(MWidget* w : m_widgets)
	{
		RenderQueue::get().removeRenderable(*w);
	}
}

//////////////////////////////////////////////////////////////////////////
// MUIManager
//////////////////////////////////////////////////////////////////////////

UIManager::UIManager()
{
	{
		ObjectPtr<MUIStyle> style = createObject<MUIStyleEmpty>();
		addStyle("empty", style);
		addStyle("default_label", style);
	}

	{
		ObjectPtr<MUIStyleFlat> style = createObject<MUIStyleFlat>();
		addStyle("default", style);
		addStyle("default_button", style);
	}

}

void UIManager::addStyle(std::string_view name, const ObjectPtr<MUIStyle>& style)
{
	m_styles.emplace_back(name, style);
}

ObjectPtr<MUIStyle> UIManager::findStyle(std::string_view name)
{
	for(auto&& p : m_styles)
	{
		if (p.first == name)
		{
			return p.second;
		}
	}

	return nullptr;
}

} // namespace mlge

