#include "mlge/Render/RenderQueue.h"
#include "mlge/Config.h"
#include "mlge/Actor.h"
#include "mlge/Profiler.h"

namespace mlge
{

//
// RenderQueue
//

bool RenderQueue::init()
{
	bool overlayDebug = Config::get().getValueOrDefault("Engine", "renderqueue_overlaydebug", false);
	m_groups[static_cast<int>(RenderGroup::OverlayDebug)].active = overlayDebug;

	return true;
}

void RenderQueue::addRenderComponent(MRenderComponent& component)
{
	m_renderables.emplace(&component);
}

void RenderQueue::removeRenderComponent(MRenderComponent& component)
{
	m_renderables.erase(&component);
}

void RenderQueue::addOp(RenderOperation& op, RenderGroup group)
{
	unsigned int idx = static_cast<unsigned int>(group);
	CZ_CHECK(idx < static_cast<unsigned int>(RenderGroup::MAX));
	m_groups[idx].ops.push_back(&op);
}

void RenderQueue::render()
{
	MLGE_PROFILE_SCOPE(mlge_RenderQueue_render);


	{
		MLGE_PROFILE_SCOPE(mlge_RenderQueue_updateRenderQueue);
		for(MRenderComponent* renderable : m_renderables)
		{
			renderable->updateRenderQueue();
		}
	}

	{
		MLGE_PROFILE_SCOPE(mlge_RenderQueue_renderOps);

		for(int idx = 0; idx<static_cast<int>(RenderGroup::MAX); idx++)
		{
			RenderGroupData& group = m_groups[idx]; 
			
			if (group.active)
			{
				for(RenderOperation* op : group.ops)
				{
					op->render(static_cast<RenderGroup>(idx));
				}
			}
			group.ops.clear();
		}
	}
}

//
// MRenderComponent
//

MRenderComponent::~MRenderComponent()
{
	// At this point the component should not be attached to any actor
	CZ_CHECK(m_owner==nullptr);
}

void MRenderComponent::setRenderGroup(
	RenderGroup renderGroup, RenderGroup
#if MLGE_DEBUG
	debugRenderGroup
#endif
)
{
	m_renderGroup = renderGroup;
#if MLGE_DEBUG
	m_debugRenderGroup = debugRenderGroup;
#endif
}

void MRenderComponent::setRelativePosition(const FPoint& position)
{
	m_relativePos = position;
}

Point MRenderComponent::calcFinalPosition() const
{
	return (m_relativePos + m_owner->getPosition()).toPoint();
}

void MRenderComponent::onAttached()
{
	Super::onAttached();
	RenderQueue::get().addRenderComponent(*this);
}

void MRenderComponent::onDetached()
{
	if (m_owner)
	{
		RenderQueue::get().removeRenderComponent(*this);
	}

	Super::onDetached();
}


void MRenderComponent::updateRenderQueue()
{
}

} // namespace mlge

