#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"
#include "mlge/Math.h"
#include "mlge/Delegates.h"

namespace mlge
{

class Game;

namespace editor
{

class GameWindow : public Window
{
  public:

	using Super = Window;
	GameWindow(Game* game, uint32_t id);
	using Super = Window;

  protected:
	virtual bool tick(float elapsedSeconds) override;
	virtual void show();

	// Game instance this window will be controlling.
	Game* m_game = nullptr;

	// Id we use to create the imgui id that allows imgui to find the window internally
	uint32_t m_id;

	bool m_resizable = false;
	bool m_hovered = false;

	// The position of the ImGui window being used to display the game.
	Point m_imGuiWindowPos;

	// This struct is used to help detect when the user stops resizing the window.
	// Instead of recreating the render target constantly, we detect when the size doesn't change for a while. After that timeout
	// we apply the new size
	struct  
	{
		// Size ImGui is reporting
		Size newSize = {};
		double applyTime = 0;
	} m_resizeCountdown;

	DelegateHandle m_onProcessEventHandle;
	void onProcessEvent(SDL_Event& evt);

	std::vector<std::string> m_resolutions;
	void showResolution(const char* resolutionStr);

	void showStopAndPause();
};

} // namespace editor
} // namespace mlge

#endif



