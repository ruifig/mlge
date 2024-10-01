
#include "mlge/Level.h"
#include "mlge/Profiler.h"
#include "crazygaze/core/Algorithm.h"

namespace mlge
{

void MLevel::removeActor(AActor* actor)
{
	std::erase_if(
		m_actors,
		[actor](const ObjectPtr<AActor>& a)
	{
		return a.get() == actor;
	});
}

void MLevel::removeAllActors()
{
	m_actors.clear();
}

void MLevel::tick(float deltaSeconds)
{
	MLGE_PROFILE_SCOPE(mlge_Level_tick);

	for(auto&& actor : m_actors)
	{
		actor->tick(deltaSeconds);
	}
}

} // namespace mlge

