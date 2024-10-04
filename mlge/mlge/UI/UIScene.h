#pragma once

#include "mlge/Object.h"
#include "mlge/UI/UIWidget.h"

namespace mlge
{

class UIManager;

MLGE_OBJECT_START(MUIScene, MObject, "Controls a series of Widgets")
class MUIScene : public MObject
{
	MLGE_OBJECT_INTERNALS(MUIScene, MObject)

  public:

	enum class State
	{
		Disabled, // Scene is not visible and is not ticked
		Enabled,  // Scene is visible and ticked but doesn't receive UI events
		Active,	  // Scene is fully active (visible, ticked, and receives events)
	};

	bool construct(UIManager& outer, std::string_view name);

	MUIWidget& getRootWidget()
	{
		return *m_rootWidget;
	}

	State getState() const
	{
		return m_state;
	}

	/**
	 * Tells transitions to the Enabled state are allowed.
	 * If now allowed, state transitions are between Disabled and Active only.
	 */
	void setCanEnable(bool canEnable)
	{
		m_canEnable = canEnable;
	}

	/**
	 * If the scene is not in Active state, it lowers the state down to Enabled or Disabled, depending on if Enabled state is
	 * allowed.
	 */
	void deactivate()
	{
		if (m_state < State::Active)
		{
			return;
		}

		setState(m_canEnable ? State::Enabled : State::Disabled);
	}

	/**
	 * Changes the state to Disabled.
	 * Depending on the current state, it will first call onDeactivate then onDisable methods.
	 */
	void disable()
	{
		if (m_state == State::Disabled)
		{
			return;
		}

		setState(State::Disabled);
	}

	virtual void tick(float deltaSeconds);

	UIManager& getManager()
	{
		return *m_outer;
	}

  protected:

	virtual bool preConstruct() override;
	virtual void destruct() override;

	// State transitions (going up)
	virtual void onEnable();
	virtual void onActivate();

	// State transitions (going down)
	virtual void onDeactivate();
	virtual void onDisable();

	void setState(State newState)
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

	ObjectPtr<MUIWidget> m_rootWidget;

	friend MUIWidget;
	friend UIManager;

	void addWidget(MUIWidget& widget);
	void removeWidget(MUIWidget& widget);
	void onUIEvent(UIEvent& evt);

	UIManager* m_outer = nullptr;
	State m_state = State::Disabled;
	std::string m_name;

	// All the widgets in this scene. This is non-owning relationship.
	// - A widget lifetime is controlled by its parent
	// - A widget adds and removes itself from this vector
	std::vector<MUIWidget*> m_widgets;

	// Tells if the scene can stay in the intermediate state "Enabled"
	bool m_canEnable = true;
};
MLGE_OBJECT_END(MUIScene)


/**
 * Controls a group of UI scenes
 */
class UIManager
{
  public:

	UIManager();

	/**
	 * Creates a MUIScene.
	 * The lifetime of the scene is controlled by the UIManager.
	 *
	 */
	template<typename SceneType, typename... Args>
	WeakObjectPtr<SceneType> createScene(Args&& ... args)
	{
		static_assert(!std::is_abstract_v<SceneType>, "Widget type is abstract");
		static_assert(std::is_base_of_v<MUIScene, SceneType>, "SceneType needs to be a MUIScene or derive from it");
		ObjectPtr<SceneType> scene = createObject<SceneType>(*this, std::forward<Args>(args)...);
		if (scene)
		{
			m_scenes.push_back(scene);
		}

		return scene;
	}

	void tick(float deltaSeconds)
	{
		for(auto&& scene : m_scenes)
		{
			if (scene->getState() >= MUIScene::State::Enabled)
			{
				scene->tick(deltaSeconds);
			}
		}
	}

	/**
	 * Activate the specified scene and deactivates all others.
	 */
	void activateScene(std::string_view name)
	{
		for(const ObjectPtr<MUIScene>& scene : m_scenes)
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

	void addStyle(std::string_view name, const ObjectPtr<MUIStyle>& style);
	ObjectPtr<MUIStyle> findStyle(std::string_view name);

  private:

	std::vector<ObjectPtr<MUIScene>> m_scenes;
	std::vector<std::pair<std::string, ObjectPtr<MUIStyle>>> m_styles;

	MUIScene* m_activeScene = nullptr;
};

} // namespace mlge

