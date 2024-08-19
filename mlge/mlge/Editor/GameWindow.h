#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"
#include "mlge/Math.h"

namespace mlge::editor
{

class GameWindow : public Window
{
  public:

	using Super = Window;
	GameWindow();
	using Super = Window;

  protected:
	virtual bool tick(float elapsedSeconds) override;
	virtual void show();

	bool m_resizable = false;


	// This struct is used to help detect when the user stops resizing the window.
	// Instead of recreating the render target constantly, we detect when the size doesn't change for a while. After that timeout
	// we apply the new size
	struct  
	{
		// Size ImGui is reporting
		Size newSize = {};
		double applyTime = 0;
	} m_resizeCountdown;
};

} // namespace mlge::editor

#endif



