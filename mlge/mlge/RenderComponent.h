#pragma once

#include "mlge/Common.h"
#include "mlge/ActorComponent.h"
#include "mlge/Render/RenderQueue.h"

namespace mlge
{

MLGE_OBJECT_START(MRenderComponent, MActorComponent, "An actor component that can be rendered.")
class MRenderComponent : public MActorComponent, public Renderable
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

  protected:

	//
	// Renderable interface
	//
	virtual void updateRenderQueue() override;
	
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
	RenderGroup m_renderGroup = RenderGroup::Main;
#if MLGE_DEBUG
	RenderGroup m_debugRenderGroup = RenderGroup::OverlayDebug;
#endif

};
MLGE_OBJECT_END(MRenderComponent)


} // namespace mlge

