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

void MUIScene::onUIEvent(UIEvent& evt)
{
	m_rootWidget->onUIEvent(evt);
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

void MUIMouseCursor::render(RenderGroup group)
{
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
	for (auto&& scene : m_scenes)
	{
		if (scene->getState() >= MUIScene::State::Enabled)
		{
			scene->tick(deltaSeconds);
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

void UIManager::onProcessEvent(SDL_Event& evt)
{
	if (evt.type == SDL_WINDOWEVENT)
	{
		if (evt.window.event == SDL_WINDOWEVENT_ENTER)
		{
			CZ_LOG(Log, "Window ENTER");
			m_mouseFocus = evt.window.windowID;
			m_mouseCursor->setEnabled(true);
		}
		else if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
		{
			CZ_LOG(Log, "Window LEAVE");
			m_mouseFocus = std::nullopt;
			m_mouseCursor->setEnabled(false);
		}
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
	#if 1
		CZ_LOG(Log, "mousemotion: timestamp={}, state={}, x={}, y={}, xrel={}, yrel={}",
			evt.motion.timestamp,
			evt.motion.state,
			evt.motion.x, evt.motion.y,
			evt.motion.xrel, evt.motion.yrel);
	#endif

		if (m_mouseFocus.has_value())
		{
			m_mouseCursor->setPosition({evt.motion.x, evt.motion.y});
			//m_mouseCursor->setPosition({evt.motion.xrel, evt.motion.yrel});
		}

	}


#if 0

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT)
		{
			if (m_rotationOn != -1)
			{
				m_angularSpeed = 0;
			}
			m_rotationOn = -1;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			if (m_rotationOn != +1)
			{
				m_angularSpeed = 0;
			}
			m_rotationOn = +1;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_UP)
		{
			m_engineOn = true;
		}

	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT || evt.key.keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			m_rotationOn = 0;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_UP)
		{
			m_engineOn = false;
		}
	}
#endif
}

} // namespace mlge

