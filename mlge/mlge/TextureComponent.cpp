#pragma once

#include "mlge/TextureComponent.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

void MTextureComponent::setTexture(ObjectPtr<MTexture> texture)
{
	m_texture = std::move(texture);
}

void MTextureComponent::updateRenderQueue()
{
	if (m_texture)
	{
		RenderQueue::get().addOp(*this, RenderGroup::Main);
	}
}

void MTextureComponent::render(RenderGroup /*group*/)
{
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	Rect srcRect({0, 0}, m_texture->getSize().w, m_texture->getSize().h);
	Rect dstRect(srcRect);
	dstRect.translate(calcFinalPosition());

	SDL_RenderCopy(sdlRenderer, m_texture->getTexture(), &srcRect, &dstRect);
}


} // namespace mlge

