#pragma once

#include "mlge/Common.h"
#include "mlge/Actor.h"

namespace mlge
{

MLGE_OBJECT_START(MLevel, MObject, "A level keeps track of all the actors." )
class MLevel : public MObject
{
	MLGE_OBJECT_INTERNALS(MLevel, MObject)

  public:

	virtual ~MLevel() = default;

	/**
	 * Creates and adds a new actor to the level
	 */
	template<typename ActorType, typename... Args>
	ObjectPtr<ActorType> addNewActor(Args&& ... args)
	{
		static_assert(!std::is_abstract_v<ActorType>, "Actor type is abstract.");
		ObjectPtr<ActorType> actor = mlge::createObject<ActorType>();
		if (actor)
		{
			m_actors.insert(actor);
		}
		return actor;
	}

	void removeActor(AActor* actor);

	virtual void tick(float deltaSeconds);

  protected:
	std::set<ObjectPtr<AActor>> m_actors;
};
MLGE_OBJECT_END(MLevel)

} // namespace mlge

