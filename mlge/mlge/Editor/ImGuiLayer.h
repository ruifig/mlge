#pragma once

#if MLGE_EDITOR

#include "mlge/Common.h"

namespace mlge::editor
{

class ImGuiLayer
{
  public:

	void init();
	void shutdown();

	void processEvent(SDL_Event& evt);
	void beginFrame();
	void endFrame();

	uint32_t getActiveWidgetId() const;

  protected:

	bool m_initialized = false;

	bool m_firstFrame = true;
	bool m_wantCaptureMouse = false;
	bool m_wantCaptureKeyboard = false;
};

} // namespace mlge::editor

#endif

