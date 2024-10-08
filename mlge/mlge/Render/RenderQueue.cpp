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

void RenderQueue::addRenderable(Renderable& renderable)
{
	m_renderables.emplace(&renderable);
}

void RenderQueue::removeRenderable(Renderable& renderable)
{
	m_renderables.erase(&renderable);
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
		for(Renderable* renderable : m_renderables)
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

} // namespace mlge

