#pragma once

#include "mlge/Common.h"
#include "mlge/ActorComponent.h"
#include "mlge/Math.h"

#include "crazygaze/core/Singleton.h"

namespace mlge
{

enum class RenderGroup
{
	Background = 0,
	SkiesEarlier = 5,
	Main = 10,
	PhysicsDebug = 20,
	SkiesLate = 21,
	Overlay = 25,
	OverlayDebug = 30,
	Stats = 31,
	MAX = 32
};

class RenderOperation
{
  public:
	virtual ~RenderOperation() {}
	virtual void render(RenderGroup group) = 0;
};

// Forward declarations
class MRenderComponent;

class RenderQueue : public Singleton<RenderQueue>
{
  public:

	bool init();

	/**
	 * Called by MRenderComponent instances when constructed, to register themselves with the render queue.
	 */
	void addRenderComponent(MRenderComponent& component);

	/**
	 * Called by MRenderComponent instances when destructed.
	 */
	void removeRenderComponent(MRenderComponent& component);

	/**
	 * Called every frame by MRenderComponent instances to add render operations to the queue.
	 */
	void addOp(RenderOperation& op, RenderGroup group);

	void render();

  private:
	std::set<MRenderComponent*> m_renderables;

	struct RenderGroupData
	{
		std::vector<RenderOperation*> ops;
		bool active = true;
	};

	RenderGroupData m_groups[static_cast<int>(RenderGroup::MAX)];
};

MLGE_OBJECT_START(MRenderComponent, MActorComponent, "An actor component that can be rendered.")
class MRenderComponent : public MActorComponent
{
	MLGE_OBJECT_INTERNALS(MRenderComponent, MActorComponent)
  public:

	virtual ~MRenderComponent();

	/**
	 * Sets the render group to use
	 *
	 * @param renderGroup Render group to use for the text
	 * @param debugRenderGroup Render group to use for any debug information
	 */
	void setRenderGroup(RenderGroup renderGroup, RenderGroup debugRenderGroup = RenderGroup::OverlayDebug);

	/**
	 * Sets the position relative to the owning Actor's position
	 */
	void setRelativePosition(const FPoint& position);

	// Component interface
	virtual void onAttached() override;
	virtual void onDetached() override;

	/**
	 * Called by RenderQueue
	 * The component should make calls to RenderQueue to add its RenderOperation objects
	 */
	virtual void updateRenderQueue();

  protected:
	
	/**
	 * Calculates the final position.
	 * This takes into account the owner's position
	 */
	Point calcFinalPosition() const;

	/**
	 * Position relative to the owner's position
	 */
	FPoint m_relativePos = {0,0};

	/**
	 * What render group to use
	 */
	RenderGroup m_renderGroup = RenderGroup::Overlay;
#if MLGE_DEBUG
	RenderGroup m_debugRenderGroup = RenderGroup::OverlayDebug;
#endif

};
MLGE_OBJECT_END(MRenderComponent)

} // namespace mlge

