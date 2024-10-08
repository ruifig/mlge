#include "mlge/UI/UIScene.h"
#include "mlge/Engine.h"
#include "mlge/Render/Renderer.h"
#include "crazygaze/core/Algorithm.h"
#include "mlge/Game.h"

namespace mlge
{


//////////////////////////////////////////////////////////////////////////
// MUIScene
//////////////////////////////////////////////////////////////////////////

bool MUIScene::preConstruct()
{
	return true;
}

bool MUIScene::construct(UIManager& outer, std::string_view name)
{
	m_outer = &outer;
	m_name = name;

	m_rootWidget = createObject<MUIWidget>(*this);
	m_rootWidget->setPosition(WidgetRect(UIUnitType::Percentage, 0, 0, 1, 1));
	m_rootWidget->setStyle(m_outer->findStyle("empty"));

	return true;
}

void MUIScene::destruct()
{
	m_rootWidget = nullptr;
}

void MUIScene::addWidget(MUIWidget& widget)
{
	m_widgets.push_back(&widget);

	if (m_state >= State::Enabled)
	{
		RenderQueue::get().addRenderable(widget);
	}
}

void MUIScene::removeWidget(MUIWidget& widget)
{
	cz::remove(m_widgets, &widget);

	if (m_state >= State::Enabled)
	{
		RenderQueue::get().removeRenderable(widget);
	}
}

void MUIScene::deactivate()
{
	if (m_state < State::Active)
	{
		return;
	}

	setState(m_canEnable ? State::Enabled : State::Disabled);
}

void MUIScene::disable()
{
	if (m_state == State::Disabled)
	{
		return;
	}

	setState(State::Disabled);
}

void MUIScene::tick(float deltaSeconds)
{
	m_rootWidget->tick(deltaSeconds);
}

void MUIScene::onEnable()
{
	for(MUIWidget* w : m_widgets)
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
	for(MUIWidget* w : m_widgets)
	{
		RenderQueue::get().removeRenderable(*w);
	}
}

void MUIScene::setState(State newState)
{
	// Going up
	if (newState > m_state)
	{
		if (m_state == State::Disabled)
		{
			m_state = State::Enabled;
			onEnable();
		}

		if (newState == State::Active)
		{
			m_state = State::Active;
			onActivate();
		}

		m_rootWidget->propagateEnabled();
	}
	// Going down
	else if (newState < m_state)
	{
		if (m_state == State::Active)
		{
			m_state = State::Enabled;
			onDeactivate();
		}

		if (newState == State::Disabled)
		{
			m_state = State::Disabled;
			onDisable();
		}

		m_rootWidget->propagateEnabled();
	}
}

void MUIMouseCursor::setEnabled(bool enabled)
{
	m_enabled = enabled;
	if (enabled)
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
}

void MUIMouseCursor::setPosition(Point pos)
{
	m_pos = pos;
}

void MUIMouseCursor::incPosition(Point inc)
{
	Size screenSize = Game::get().getRenderTarget().getSize();

	m_pos.x += inc.x;
	if (m_pos.x < 0)
	{
		m_pos.x = 0;
	}
	else if (m_pos.x >= screenSize.w)
	{
		m_pos.x = screenSize.w - 1;
	}

	m_pos.y += inc.y;
	if (m_pos.y < 0)
	{
		m_pos.y = 0;
	}
	else if (m_pos.y >= screenSize.h)
	{
		m_pos.y = screenSize.h - 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// MUIMouseCursor
//////////////////////////////////////////////////////////////////////////

void MUIMouseCursor::postConstruct()
{
	m_spriteSheet = ms_spriteSheetRef.getResource();
	
	if (m_spriteSheet)
	{
		RenderQueue::get().addRenderable(*this);
	}
}

void MUIMouseCursor::destruct()
{
	RenderQueue::get().removeRenderable(*this);
}

void MUIMouseCursor::updateRenderQueue()
{
	if (!m_enabled)
	{
		return;
	}

	RenderQueue::get().addOp(*this, RenderGroup::MouseCursor);
}

void MUIMouseCursor::render(RenderGroup /*group*/)
{
	//CZ_LOG(Log, "Mouse render");
	renderSprite(m_spriteSheet->getSprite(0), m_pos);
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

	m_onProcessEventHandle = Engine::get().processEventDelegate.bind(this, &UIManager::onProcessEvent);
	m_onWindowResizedHandle = Game::get().windowResizedDelegate.bind(this, &UIManager::onWindowResized);
	m_onWindowEnterHandle = Game::get().windowEnterDelegate.bind(this, &UIManager::onWindowEnter);
	m_onMouseMotionHandle = Game::get().mouseMotionDelegate.bind(this, &UIManager::onMouseMotion);
	m_onWindowFocusHandle = Game::get().windowFocus.bind(this, &UIManager::onWindowFocus);

	m_mouseCursor = createObject<MUIMouseCursor>();

	//SDL_SetRelativeMouseMode(SDL_TRUE);
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

void UIManager::tick(float deltaSeconds)
{
	m_eventStack.clear();

	for (auto&& scene : m_scenes)
	{
		if (scene->getState() >= MUIScene::State::Enabled)
		{
			scene->tick(deltaSeconds);
			if (m_mouseWindowFocus)
			{
				scene->getRootWidget().processMouseCursor(m_mouseCursor->getPosition(), m_eventStack);
			}
		}
	}

}

void UIManager::activateScene(std::string_view name)
{
	for (const ObjectPtr<MUIScene>& scene : m_scenes)
	{
		if (scene->m_name == name)
		{
			// Nothing to do
			if (scene->m_state == MUIScene::State::Active)
			{
				CZ_LOG(Verbose, "Scene '{}' already active.", name);
				return;
			}
		}

		// First we deactivate the currently active scene
		if (m_activeScene)
		{
			CZ_CHECK(m_activeScene->m_state == MUIScene::State::Active);
			CZ_LOG(Log, "Deactivating scene '{}'.", m_activeScene->m_name);
			m_activeScene->setState(m_activeScene->m_canEnable ? MUIScene::State::Enabled : MUIScene::State::Disabled);
		}

		CZ_LOG(Log, "Activating scene '{}'.", scene->m_name);
		scene->setState(MUIScene::State::Active);
		m_activeScene = scene.get();
	}
}

void UIManager::onWindowEnter(bool entered)
{
	if (entered)
	{
		m_mouseWindowFocus = true;
		m_mouseCursor->setEnabled(true);
	}
	else
	{
		m_mouseWindowFocus = false;
		m_mouseCursor->setEnabled(false);

		// Process all widgets considering a position outside the screen.
		// This makes sure that the onMouseLeave methods are called when the mouse moves outside the window
		{
			m_eventStack.clear();
			for (auto&& scene : m_scenes)
			{
				if (scene->getState() >= MUIScene::State::Enabled)
				{
					scene->getRootWidget().processMouseCursor({-1,-1}, m_eventStack);
				}
			}
		}
	}
}

void UIManager::onWindowResized(Size /*newSize*/)
{
	for (const ObjectPtr<MUIScene>& scene : m_scenes)
	{
		scene->getRootWidget().onWindowResized();
	}
}

void UIManager::onMouseMotion(const Game::MouseMotionEvent& evt)
{
	if (m_mouseWindowFocus)
	{
		m_mouseCursor->setPosition({evt.pos.x, evt.pos.y});
		//m_mouseCursor->setPosition({evt.motion.xrel, evt.motion.yrel});
	}
}

void UIManager::onWindowFocus(bool focus)
{
	m_mouseCursor->setEnabled(focus);
}

void UIManager::onProcessEvent(SDL_Event& evt)
{
	if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
	{
		
		auto raiseEvent = [this](UIInternalEvent::Type type, const Point& pos)
		{
			UIInternalEvent e;
			e.type = type;
			e.pos = pos;

			#if 0
			CZ_LOG(VeryVerbose, "EventStack:{}", m_eventStack.size());
			for (auto w : m_eventStack)
			{
				CZ_LOG(VeryVerbose, "    {}", w->getObjectName());
			}
			#endif

			for (auto it = m_eventStack.rbegin(); it != m_eventStack.rend(); it++)
			{
				(*it)->onUIInternalEvent(e);
				if (e.consumed)
				{
					break;
				}
			}
		};

		//CZ_LOG(Log, "Mouse: button {}, state {}, numclicks {}", evt.button.button, evt.button.state, evt.button.clicks);
		if (evt.button.button == 1 && m_eventStack.size())
		{
			UIInternalEvent::Type type =
				evt.button.state == SDL_PRESSED ? UIInternalEvent::Type::Pressed : UIInternalEvent::Type::Released;

			bool raiseClick = false;
			if (type == UIInternalEvent::Type::Pressed)
			{
				m_pressedWidget = m_eventStack.back();
			}
			else if (type == UIInternalEvent::Type::Released)
			{
				if (m_eventStack.back() == m_pressedWidget)
				{
					raiseClick = true;
				}
			}

			raiseEvent(type, m_mouseCursor->getPosition());

			if (raiseClick)
			{
				raiseEvent(UIInternalEvent::Type::Click, m_mouseCursor->getPosition());
			}

		}
	}
}

} // namespace mlge

