#include "mlge/FlipbookComponent.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Actor.h"

namespace mlge
{

void MFlipbookComponent::setFlipbook(const ObjectPtr<MFlipbook>& flipbook)
{
	m_flipbook = flipbook;
	m_pos = m_flipbook->getTimelineStartPosition();
}

void MFlipbookComponent::tick(float elapsedSeconds)
{
	if (!m_flipbook)
	{
		return;
	}

	m_pos.tick(elapsedSeconds);
}

void MFlipbookComponent::updateRenderQueue()
{
	if (m_flipbook)
	{
		RenderQueue::get().addOp(*this, RenderGroup::Main);
	}
}

void MFlipbookComponent::render(RenderGroup /*group*/)
{
	renderSprite(m_flipbook->getSpriteAndUpdatePosition(m_pos), calcFinalPosition(), m_owner->getRotation(), 1.0f);
}


} // namespace mlge
