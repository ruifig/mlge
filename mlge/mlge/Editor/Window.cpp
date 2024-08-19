#if MLGE_EDITOR

#include "mlge/Editor/Window.h"

namespace mlge::editor
{

Window::Window()
{
}

Window::~Window()
{
}

Window::Window(bool& p_open, const std::string& title)
	: m_p_open(&p_open)
	, m_title(title)
{
}

bool Window::tick(float /*elapsedSeconds*/)
{
	if (!ImGui::Begin(m_title.c_str(), m_p_open))
	{
		ImGui::End();
		return false;
	}

	show();

	ImGui::End();
	return m_p_open ? *m_p_open : true;
}

} // namespace mlge::editor

#endif

