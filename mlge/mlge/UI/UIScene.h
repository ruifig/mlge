#pragma once

#include "mlge/Object.h"
#include "mlge/UI/UIWidget.h"
#include "mlge/Delegates.h"
#include "mlge/Resource/SpriteSheet.h"

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
	void deactivate();

	/**
	 * Changes the state to Disabled.
	 * Depending on the current state, it will first call onDeactivate then onDisable methods.
	 */
	void disable();

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

	void setState(State newState);

	ObjectPtr<MUIWidget> m_rootWidget;

	friend MUIWidget;
	friend UIManager;

	void addWidget(MUIWidget& widget);
	void removeWidget(MUIWidget& widget);

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

MLGE_OBJECT_START(MUIMouseCursor, MObject, "Tracks and renders the mouse cursor")
class MUIMouseCursor : public MObject , public Renderable, public RenderOperation
{
	MLGE_OBJECT_INTERNALS(MUIMouseCursor, MObject)
  public:

 
  protected:

	friend UIManager;

	virtual void setEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	const Point& getPosition() const
	{
		return m_pos;
	}
	virtual void setPosition(Point pos);

	virtual void incPosition(Point inc);

  private:

	virtual void postConstruct();
	virtual void destruct();
	
	// Renderable interface
	virtual void updateRenderQueue() override;

	// RenderOperation interface
	virtual void render(RenderGroup group) override;

	bool m_enabled = false;
	Point m_pos = {};
	inline static StaticResourceRef<MSpriteSheet> ms_spriteSheetRef = "cursors/default";
	ObjectPtr<MSpriteSheet> m_spriteSheet;
};
MLGE_OBJECT_END(MUIMouseCursor)

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

	void tick(float deltaSeconds);

	void showMouseCursor(bool show)
	{
		m_mouseCursor->setEnabled(show);
		SDL_ShowCursor(SDL_DISABLE);
	}

	/**
	 * Activate the specified scene and deactivates all others.
	 */
	void activateScene(std::string_view name);

	void addStyle(std::string_view name, const ObjectPtr<MUIStyle>& style);
	ObjectPtr<MUIStyle> findStyle(std::string_view name);

	MultiCastDelegate<const UIEvent&> uiEventDelegate;

  private:

	std::vector<ObjectPtr<MUIScene>> m_scenes;
	std::vector<std::pair<std::string, ObjectPtr<MUIStyle>>> m_styles;

	ObjectPtr<MUIMouseCursor> m_mouseCursor;

	// WindowID with mouse focus. If not set, then mouse is not focus on the window
	std::optional<uint32_t> m_mouseFocus;
	MUIScene* m_activeScene = nullptr;

	std::vector<MUIWidget*> m_eventStack;

	DelegateHandle m_onProcessEventHandle;
	void onProcessEvent(SDL_Event& evt);

	DelegateHandle m_onWindowResizedHandle;
	void onWindowResized(Size newSize);
};

} // namespace mlge

