#pragma once

#if MLGE_EDITOR

#include "mlge/Common.h"

namespace mlge::editor
{

class Window
{
  public:
	
	CZ_DELETE_COPY_AND_MOVE(Window);
	Window(bool& p_open, const std::string& title);
	Window();
	virtual ~Window();

	/**
	 * Ticks the window.
	 * The window should return true if it wishes to continue alive, or false if it should be destroyed.
	 */
	virtual bool tick(float /*elapsedSeconds*/);

	void setTag(void* tag)
	{
		m_tag = tag;
	}

	void* getTag() const
	{
		return m_tag;
	}

  protected:

	bool* m_p_open = nullptr;
	std::string m_title;
	// Any code creating windows can set this to identify a window
	void* m_tag = nullptr;
	virtual void show() = 0;
};

} // namespace mlge::editor

#endif

