#pragma once

#include "mlge/Common.h"
#include "mlge/ActorComponent.h"
#include "mlge/Math.h"
#include "mlge/Misc/ContainerUtils.h"
#include "mlge/Misc/LinkedList.h"
#include "mlge/Object.h"
#include "crazygaze/core/Logging.h"

namespace mlge
{

MLGE_OBJECT_START(AActor, MObject, "Base Actor class.\nActors can be placed in a level")

class AActor : public MObject
{
	MLGE_OBJECT_INTERNALS(AActor, MObject)

  public:
	virtual ~AActor();
	virtual void tick(float deltaSeconds);

	const FPoint& getPosition() const
	{
		return m_position;
	}

	void setPosition(FPoint pos)
	{
		m_position = pos;
	}

	float getRotation() const
	{
		return m_rotation;
	}

	/**
	 *  Sets the actor's rotation, ignoring multiple revolutions. As-in, the saved rotation will stay within [-360...360]
	 */
	void setRotation(float degrees)
	{
		if (degrees >= -360 && degrees <= 360)
		{
			m_rotation = degrees;
		}
		else
		{
			m_rotation = static_cast<float>(fmod(degrees, 360));
		}
	}

	/**
	 * Creates and attaches a dynamically created component
	 */
	template<typename ComponentType, typename... Args>
	ObjectPtr<ComponentType> addNewComponent(Args&& ... args)
	{
		ObjectPtr<ComponentType> comp = createObject<ComponentType>(std::forward<Args>(args)...);

		comp->m_owner = this;
		m_components.insert(comp);
		comp->onAttached();
		return comp;
	}

	/**
	 * Detaches a component from the actor.
	 */
	void detach(MActorComponent& comp);

  protected:

	virtual void destruct() override;

  private:

	/**
	 * Actor position
	 */
	FPoint m_position = {0, 0};

	/**
	 * Actor rotation in degrees
	 */
	float m_rotation = 0;

	/**
	 * All the components attached to the actor
	 */
	std::set<ObjectPtr<MActorComponent>, details::pointer_comp<MActorComponent>> m_components;
};
MLGE_OBJECT_END(AActor)

} // namespace mlge

