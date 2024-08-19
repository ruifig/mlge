#pragma once

#include "mlge/Common.h"
#include "mlge/Object.h"

namespace mlge
{

class AActor;

MLGE_OBJECT_START(MActorComponent, MObject, "Base actor component")

class MActorComponent : public MObject
{
	MLGE_OBJECT_INTERNALS(MActorComponent, MObject)

  public:

	friend AActor;

	MActorComponent() = default;
	virtual ~MActorComponent() = default;

	virtual void tick(float /*deltaSeconds*/) { }

	bool isAttached() const
	{
		return m_owner == nullptr ? false : true;
	}

	/**
	 * Called when the component is attached to an actor
	 *
	 * This is called after m_owner is set.
	 */
	virtual void onAttached() {}

	/**
	 * Called with the component is detached from the actor
	 * This is called before m_owner is to nullptr
	 */
	virtual void onDetached() {}

  protected:

	/**
	 * Actor this component is attached to or nullptr if not attached
	 */
	AActor* m_owner = nullptr;

};
MLGE_OBJECT_END(MActorComponent)


} // namespace mlge

