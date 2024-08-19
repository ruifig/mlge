#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"
#include "mlge/Math.h"

namespace mlge::editor
{

class GameControlBar : public Window
{
  public:

	GameControlBar();
	using Super = Window;

  protected:

	virtual bool tick(float deltaSeconds) override;
	virtual void show() override;

	int m_resolutionIdx = -1;
	Size m_resolution;

	std::vector<std::string> m_resolutions;
};

} // mlge::editor

#endif
