#if MLGE_EDITOR

#include "mlge/Editor/Console.h"

#include "crazygaze/core/LogOutputs.h"

namespace mlge::editor
{

namespace
{
    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }
}

Console::Console(bool& p_open)
	: Window(p_open, "Logging & Command console")
{
	memset(m_inputBuf, 0, sizeof(m_inputBuf));

	m_commands.push_back("Help");
	m_commands.push_back("History");
	m_commands.push_back("Clear");

	addLog("Welcome to mlge (My Little Game Engine)");

	LogOutputs::get().add(this, [this](LogLevel level, const char* category, const char* timestamp, const char* msg)
	{
		auto str = std::string(timestamp) + ":" + category + ":" + to_string(level) + ": " + msg;
		m_lines.push_back(std::make_pair(level, std::move(str)));
	});

}

Console::~Console()
{
	LogOutputs::get().remove(this);
}

void Console::clearLog()
{
	m_lines.clear();
}

void Console::show()
{
	// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
	// So e.g. IsItemHovered() will return true when hovering the title bar.
	// Here we create a context menu only available from the title bar.
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Close Console"))
		{
			*m_p_open = false;
		}
		ImGui::EndPopup();
	}

	// Options
	if (ImGui::Button("Options"))
	{
		ImGui::OpenPopup("Options");
	}
	// Options menu
	if (ImGui::BeginPopup("Options"))
	{
		ImGui::Checkbox("Auto-scroll", &m_autoScroll);
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Log Level"))
	{
		ImGui::OpenPopup("Log_Level");
	}
	if (ImGui::BeginPopup("Log_Level"))
	{
		for (int i = static_cast<int>(LogLevel::Fatal); i <= static_cast<int>(LogLevel::VeryVerbose); i++)
		{
			bool selected = LogLevel(i) == details::currMaxLogLevel; 
			if (ImGui::MenuItem(to_string(LogLevel(i)), "", &selected))
			{
				details::currMaxLogLevel = LogLevel(i);
				CZ_LOG(Error, "Hello world");
				CZ_LOG(Warning, "Hello world");
				CZ_LOG(Log, "Hello world");
				CZ_LOG(Verbose, "Hello world");
				CZ_LOG(VeryVerbose, "Hello world");
			}
		}

		ImGui::EndPopup();
	}


	ImGui::SameLine();
	bool copy_to_clipboard = ImGui::SmallButton("Copy");
	ImGui::SameLine();
	m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::Separator();

	// Reserve enough left-over height for 1 separator + 1 input text
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild(
			"ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear"))
			{
				clearLog();
			}
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets.
		// If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
		// to only process visible items. The clipper will automatically measure the height of your first item and then
		// "seek" to display only items in the visible area.
		// To use the clipper we can replace your standard loop:
		//      for (int i = 0; i < Items.Size; i++)
		//   With:
		//      ImGuiListClipper clipper;
		//      clipper.Begin(Items.Size);
		//      while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// - That your items are evenly spaced (same height)
		// - That you have cheap random access to your elements (you can access them given their index,
		//   without processing all the ones before)
		// You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
		// We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
		// or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage
		// to improve this example code!
		// If your items are of variable height:
		// - Split them into same height items would be simpler and facilitate random-seeking into your list.
		// - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing
		if (copy_to_clipboard)
		{
			ImGui::LogToClipboard();
		}

		for (const std::pair<LogLevel, std::string>& line : m_lines)
		{
			const char* item = line.second.c_str();

			if (!m_filter.PassFilter(item))
			{
				continue;
			}

			// Normally you would store more information in your item than just a string.
			// (e.g. make Items[] an array of structure, store color/type etc.)
			ImVec4 color;
			bool has_color = true;
			switch(line.first)
			{
				case LogLevel::Off:
					has_color = false;
					break;
				case LogLevel::Fatal:
				case LogLevel::Error:
					color = ImColor(197, 15, 31, 255);
					break;
				case LogLevel::Warning:
					color = ImColor(193, 156, 0, 255);
					break;
				case LogLevel::Log:
					color = ImColor(19, 161, 14, 255);
					break;
				case LogLevel::Verbose:
					color = ImColor(97, 214, 214, 255);
					break;
				case LogLevel::VeryVerbose:
					color = ImColor(58, 150, 221, 255);
					break;
			}

			//if (strstr(item, "[error]"))
			//{
			//	color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
			//	has_color = true;
			//}
			//else 
			if (strncmp(item, "# ", 2) == 0)
			{
				color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
				has_color = true;
			}

			if (has_color)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, color);
			}
			ImGui::TextUnformatted(item);
			if (has_color)
			{
				ImGui::PopStyleColor();
			}
		}

		if (copy_to_clipboard)
		{
			ImGui::LogFinish();
		}

		// Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
		// Using a scrollbar or mouse-wheel will take away from the bottom edge.
		if (m_scrollToBottom || (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		{
			ImGui::SetScrollHereY(1.0f);
		}

		m_scrollToBottom = false;

		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::Separator();

	// Command-line
	bool reclaim_focus = false;
	ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll |
										   ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
	if (ImGui::InputText("Input", m_inputBuf, IM_ARRAYSIZE(m_inputBuf), input_text_flags, &textEditCallbackStub, (void*)this))
	{
		char* s = m_inputBuf;
		Strtrim(s);
		if (s[0])
		{
			execCommand(s);
		}

		strcpy(s, "");
		reclaim_focus = true;
	}

	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
	{
		ImGui::SetKeyboardFocusHere(-1);  // Auto focus previous widget
	}
}

bool Console::tick(float deltaSeconds)
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	return Super::tick(deltaSeconds);
}

int Console::textEditCallbackStub(ImGuiInputTextCallbackData* data)
{
	Console* console = (Console*)data->UserData;
	return console->textEditCallback(data);
}

int Console::textEditCallback(ImGuiInputTextCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (size_t i = 0; i < m_commands.size(); i++)
				if (Strnicmp(m_commands[i].c_str(), word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(m_commands[i].c_str());

			if (candidates.Size == 0)
			{
				// No match
				addLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can..
				// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				addLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					addLog("- %s\n", candidates[i]);
			}

			break;
		}
	case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = m_historyPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (m_historyPos == -1)
					m_historyPos = static_cast<int>(m_history.size()) - 1;
				else if (m_historyPos > 0)
					m_historyPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (m_historyPos != -1)
					if (++m_historyPos >= static_cast<int>(m_history.size()))
						m_historyPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != m_historyPos)
			{
				const char* history_str = (m_historyPos >= 0) ? m_history[(size_t)m_historyPos].c_str() : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
	}
	return 0;
}

void Console::execCommand(const char* command_line)
{
	addLog("# %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back.
	// This isn't trying to be smart or optimal.
	m_historyPos = -1;
	for (int i = static_cast<int>(m_history.size()) - 1; i >= 0; i--)
	{
		if (Stricmp(m_history[size_t(i)].c_str(), command_line) == 0)
		{
			m_history.erase(m_history.begin() + i);
			break;
		}
	}
	
	m_history.push_back(command_line);

	// Process command
	if (Stricmp(command_line, "CLEAR") == 0)
	{
		clearLog();
	}
	else if (Stricmp(command_line, "HELP") == 0)
	{
		addLog("Commands:");
		for (size_t i = 0; i < m_commands.size(); i++)
		{
			addLog("- %s", m_commands[i].c_str());
		}
	}
	else if (Stricmp(command_line, "HISTORY") == 0)
	{
		int first = static_cast<int>(m_history.size()) - 10;
		for (int i = first > 0 ? first : 0; i < static_cast<int>(m_history.size()); i++)
		{
			addLog("%3d: %s\n", i, m_history[size_t(i)].c_str());
		}
	}
	else
	{
		addLog("Unknown command: '%s'\n", command_line);
	}

	// On command input, we scroll to bottom even if AutoScroll==false
	m_scrollToBottom = true;
}

void Console::addLog(std::string_view str)
{
	m_lines.push_back(std::make_pair(LogLevel::Off, std::string(str)));
}

void Console::addLog(const char* fmt, ...) IM_FMTARGS(2)
{
	// FIXME-OPT
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf)-1] = 0;
	va_end(args);
	m_lines.push_back(std::make_pair(LogLevel::Off, std::string(buf)));
}

} // namespace mlge::editor


#endif

