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
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	const Sprite& sprite = m_flipbook->getSpriteAndUpdatePosition(m_pos);
	Rect dstRect = sprite.rect;
	dstRect.move(calcFinalPosition());

	SDL_RenderCopyEx(sdlRenderer, sprite.texture,
		&sprite.rect, &dstRect,
		m_owner->getRotation(),
		nullptr, // Rotation center
		SDL_FLIP_NONE
		);
}


} // namespace mlge
