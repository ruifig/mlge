
#if MLGE_EDITOR

#include "mlge/Editor/GameControlBar.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Editor/Editor.h"
#include "mlge/Config.h"

namespace mlge::editor
{

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

	for (size_t n = 0; n < m_resolutions.size(); n++)
	{
		if (cfgRes == m_resolutions[n])
		{
			m_resolutionIdx = static_cast<int>(n);
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
	auto getSize = [](const std::string& str)
	{
		return Size{
			atoi(std::string(str, 0, str.find('x')).c_str()),
			atoi(std::string(str, str.find('x')+1).c_str())};
	};

	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

	// Game count
	{
		ImGui::Text("Game count:");
		ImGui::SameLine();
		if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		{
			if (m_numGames > 1)
			{
				m_numGames--;
			}
		}
		ImGui::SameLine(0.0f, spacing);
		ImGui::Text("%u", m_numGames);
		ImGui::SameLine(0.0f, spacing);
		if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		{
			if (m_numGames < 10)
			{
				m_numGames++;
			}
		}

	}

	// Resolution selection
	{
		ImGui::SameLine(0.0f, spacing);

		const char* combo_preview_value =
			m_resolutions[static_cast<size_t>(m_resolutionIdx)].c_str();  // Pass in the preview value visible before opening the combo (it could be anything)

		if (ImGui::BeginCombo("Resolution", combo_preview_value, 0))
		{
			for (size_t n = 0; n < m_resolutions.size(); n++)
			{
				const bool isSelected = (m_resolutionIdx == static_cast<int>(n));
				if (ImGui::Selectable(m_resolutions[n].c_str(), isSelected))
				{
					int newIdx = static_cast<int>(n);
					auto newResolution = getSize(m_resolutions[static_cast<size_t>(newIdx)]);

					Config::get().setGameValue("Engine", "resx", newResolution.w);
					Config::get().setGameValue("Engine", "resy", newResolution.h);
					Config::get().save();

					if (Editor::get().getGamesCount())
					{
						Editor::get().visitGames([&](Game& game)
						{
							RenderTarget& renderTarget = game.getRenderTarget();
							if (renderTarget.getSize() != newResolution)
							{
								m_resolution = newResolution;
								m_resolutionIdx = newIdx;
								game.onWindowResized(newResolution);
							}
						});
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
		if (Editor::get().getGamesCount() > 0)
		{
			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				Editor::get().stopGame();
			}

			if (Editor::get().getGameAtIndex(0).isGameClockPaused())
			{
				ImGui::SameLine();
				if (ImGui::Button("Resume"))
				{
					Editor::get().visitGames([&](Game& game)
					{
						game.resumeGameClock();
					});
				}
			}
			else
			{
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
				{
					Editor::get().visitGames([&](Game& game)
					{
						game.pauseGameClock();
					});
				}
			}
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				Editor::get().startGame(m_numGames);
			}
		}
	}

	// Tip about how to get out of game focus
	if (Editor::get().anyGameHasFocus())
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
