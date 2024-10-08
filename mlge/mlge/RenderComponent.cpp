#include "RenderComponent.h"
#include "Actor.h"

namespace mlge
{

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
	RenderQueue::get().addRenderable(*this);
}

void MRenderComponent::onDetached()
{
	if (m_owner)
	{
		RenderQueue::get().removeRenderable(*this);
	}

	Super::onDetached();
}


void MRenderComponent::updateRenderQueue()
{
}


} // namespace mlge

