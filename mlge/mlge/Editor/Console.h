#pragma once

#if MLGE_EDITOR

#include "mlge/Editor/Window.h"

#include "crazygaze/core/Singleton.h"

namespace mlge::editor
{


/**
 * Logging and command console
 * Based on ImgGui console demo
 */
class Console : public Window, public Singleton<Console>
{
  public:
	Console(bool& p_open);
	~Console();
	using Super = Window;

  protected:

	virtual bool tick(float deltaSeconds) override;	
	virtual void show() override;
	void addLog(std::string_view str);
	void addLog(const char* fmt, ...) IM_FMTARGS(2);
	void clearLog();
	static int textEditCallbackStub(ImGuiInputTextCallbackData* data);
	int textEditCallback(ImGuiInputTextCallbackData* data);
	void execCommand(const char* command_line);

	char m_inputBuf[256];
	std::vector<std::pair<LogLevel,std::string>> m_lines;
	std::vector<std::string> m_commands;
	std::vector<std::string> m_history;
	ImGuiTextFilter m_filter;
	int m_historyPos = -1;
	bool m_autoScroll = true;
	bool m_scrollToBottom = false;
};


} // namespace mlge

#endif

