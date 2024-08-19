#pragma once

#include "mlge/Common.h"
#include "mlge/Math.h"

namespace mlge
{

class RenderTarget
{
  public:

	virtual ~RenderTarget()
	{
		if (ms_current == this)
		{
			ms_current = nullptr;
		}
	}

	virtual bool init(Size size) = 0;

	virtual bool setSize(Size size)
	{
		m_size = size;
		return true;
	}

	const Size& getSize() const
	{
		return m_size;
	}

	int getWidth() const
	{
		return m_size.w;
	}

	int getHeight() const
	{
		return m_size.h;
	}

	virtual SDL_Texture* getTexture()
	{
		return nullptr;
	}

  protected:

	inline static RenderTarget* ms_current = nullptr;
	Size m_size;
};

class WindowRenderTarget : public RenderTarget
{
  public:
	using Super = RenderTarget;

	virtual bool init(Size size) override;
	virtual bool setSize(Size size) override;
};

class TextureRenderTarget : public RenderTarget
{
  public:
	using Super = RenderTarget;

	virtual bool init(Size size) override;
	virtual bool setSize(Size size) override;
	virtual SDL_Texture* getTexture() override;

	SDLUniquePtr<SDL_Texture> m_texture;
};

} // namespace mlge
