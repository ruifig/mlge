#include "mlge/Render/RenderTarget.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

//////////////////////////////////////////////////////////////////////////
//		WindowRenderTarget
//////////////////////////////////////////////////////////////////////////

bool WindowRenderTarget::init(Size size)
{
	return setSize(size);
}

bool WindowRenderTarget::setSize(Size size)
{
	Size currentSize = {};
	SDL_GetWindowSize(Renderer::get().getSDLWindow(), &currentSize.w, &currentSize.h);

	if (currentSize != size)
	{
		SDL_SetWindowSize(Renderer::get().getSDLWindow(), size.w, size.h);
	}

	Super::setSize(size);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//		TextureRenderTarget
//////////////////////////////////////////////////////////////////////////

bool TextureRenderTarget::init(Size size)
{
	return setSize(size);
}

bool TextureRenderTarget::setSize(Size size)
{
	if (size == m_size)
	{
		return true;
	}

	SDLUniquePtr<SDL_Texture> tmp(SDL_CreateTexture(Renderer::get().getSDLRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.w, size.h));

	if (tmp)
	{
		Super::setSize(size);
		std::swap(tmp, m_texture);
	}
	else
	{
		CZ_LOG(Error, "Failed to create TextureRenderTarget. ec={}", SDL_GetError());
		return false;
	}

	return true;
}

SDL_Texture* TextureRenderTarget::getTexture()
{
	return m_texture.get();
}

} // namespace mlge

