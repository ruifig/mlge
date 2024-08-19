#include "mlge/Actor.h"
#include "mlge/Profiler.h"
#include "crazygaze/core/Logging.h"

namespace mlge
{

AActor::~AActor()
{
}

void AActor::tick(float deltaSeconds)
{
	MLGE_PROFILE_SCOPE(mlge_Actor_tick);

	for(auto&& component : m_components)
	{
		component->tick(deltaSeconds);
	}
}

void AActor::detach(MActorComponent& comp)
{
	CZ_CHECK(comp.m_owner == this);
	comp.onDetached();
	comp.m_owner = nullptr;
	
	// Delete dynamic component (if this is a dynamic component)
	auto it = m_components.find(&comp);
	if (it != m_components.end())
	{
		m_components.erase(it);
	}
}

void AActor::destruct()
{
	for(auto&& c : m_components)
	{
		c->onDetached();
		c->m_owner = nullptr;
	}

	m_components.clear();

	Super::destruct();
}

} // namespace mlge

