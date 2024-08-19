
#if MLGE_EDITOR

#include "mlge/Editor/GameControlBar.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Editor/Editor.h"
#include "mlge/Config.h"

namespace mlge::editor
{

namespace 
{
	const char* items[] = {"320x240", "640x480", "1024x768"};
}

static bool p_open = true;

GameControlBar::GameControlBar()
	: Window(p_open, "Game Control Bar")
{
	m_resolutions.push_back("320x240");
	m_resolutions.push_back("640x480");
	m_resolutions.push_back("1024x768");

	Size s;
	s.w = Config::get().getValueOrDefault<int>("Engine", "resx", 640);
	s.h = Config::get().getValueOrDefault<int>("Engine", "resy", 480);

	std::string cfgRes = std::format("{}x{}", s.w, s.h);

	for (int n = 0; n < IM_ARRAYSIZE(items); n++)
	{
		if (cfgRes == items[n])
		{
			m_resolutionIdx = n;
			break;
		}
	}

	if (m_resolutionIdx == -1)
	{
		m_resolutionIdx = static_cast<int>(m_resolutions.size());
		m_resolutions.push_back(cfgRes);
	}
}

void GameControlBar::show()
{
	auto getSize = [](std::string str)
	{
		return Size{
			atoi(std::string(str, 0, str.find('x')).c_str()),
			atoi(std::string(str, str.find('x')+1).c_str())};
	};

	// Resolution selection
	{
		const char* combo_preview_value =
			items[m_resolutionIdx];  // Pass in the preview value visible before opening the combo (it could be anything)
		if (ImGui::BeginCombo("Resolution", combo_preview_value, 0))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				const bool isSelected = (m_resolutionIdx == n);
				if (ImGui::Selectable(items[n], isSelected))
				{
					int newIdx = n;
					auto newResolution = getSize(items[newIdx]);

					Config::get().setGameValue("Engine", "resx", newResolution.w);
					Config::get().setGameValue("Engine", "resy", newResolution.h);
					Config::get().save();

					if (Game::tryGet())
					{
						RenderTarget& renderTarget = Game::get().getRenderTarget();
						if (renderTarget.getSize() != newResolution && renderTarget.setSize(newResolution))
						{
							m_resolution = newResolution;
							m_resolutionIdx = newIdx;
						}
					}
					else
					{
						m_resolution = newResolution;
						m_resolutionIdx = newIdx;
					}
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	// Play stop buttons
	{
		bool hasGame = Game::tryGet()==nullptr ? false : true;

		if (hasGame)
		{
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				Editor::get().stopGame();
			}

			if (Game::get().isGameClockPaused())
			{
				ImGui::SameLine();
				if (ImGui::Button("Resume"))
				{
					Game::get().resumeGameClock();
				}
			}
			else
			{
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
				{
					Game::get().pauseGameClock();
				}
			}
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				Editor::get().startGame();
			}
		}
	}

	// Tip about how to get out of game focus
	if (Editor::get().gameHasFocus())
	{
		ImGui::SameLine();
		ImGui::Text("Press ALT to release focus from game");
	}

}

bool GameControlBar::tick(float /*deltaSeconds*/)
{
	ImGui::Begin("GameControlBar", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	show();
	ImGui::End();
	return true;
}


} // mlge::editor

#endif
