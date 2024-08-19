#pragma  once

#include "mlge/Common.h"
#include "mlge/Render/RenderTarget.h"
#include "mlge/Delegates.h"
#include "crazygaze/core/Semaphore.h"
#include "crazygaze/core/Singleton.h"

namespace mlge
{

class Renderer : public Singleton<Renderer>
{
  public:

	Renderer();
	~Renderer();

	/**
	 * Initialized the required SDL components (e.g: Window & Renderer)
	 */
	bool init();

	/**
	 * Sets the render target to use
	 *
	 * @param render target to use, or nullptr to use the window render target
	 */
	void setTarget(RenderTarget* target);

	/**
	 * Clears the render target with the specified color
	 */
	void clearTarget(Color clearColor);

	/**
	 * Called by the game loop before ticking the game
	 */
	void beginFrame();

	/**
	 * Starts the rendering operations
	 * This actually transfers control to the render thread and blocks until the render thread has done all it needs with
	 * the shared state. This means that from the game's perspective, everything is still single threaded.
	 */
	void render();

	SDL_Window* getSDLWindow()
	{
		return m_sdlWindow.get();
	}

	SDL_Renderer* getSDLRenderer()
	{
		return m_sdlRenderer.get();
	}

	void shutdown();

	uint32_t getFrameNumber() const
	{
		return m_frameNumber;
	}

	MultiCastDelegate<> beginFrameDelegate;
	MultiCastDelegate<> endFrameDelegate;
	MultiCastDelegate<> gameRenderFinishedDelegate;

  protected:

	void draw();

	SDLUniquePtr<SDL_Window> m_sdlWindow;
	SDLUniquePtr<SDL_Renderer> m_sdlRenderer;
	RenderTarget* m_renderTarget = nullptr;
	uint32_t m_frameNumber = 0;
};

} // namespace mlge

