#if MLGE_EDITOR

#include "mlge/Editor/GameWindow.h"
#include "mlge/Editor/Editor.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Profiler.h"
#include "mlge/Engine.h"
#include "mlge/Game.h"
#include "crazygaze/core/Algorithm.h"

namespace mlge::editor
{

GameWindow::GameWindow(Game* game, uint32_t id)
	: m_game(game)
	, m_id(id)
{
	m_resolutions.push_back("320x240");
	m_resolutions.push_back("640x480");
	m_resolutions.push_back("1024x768");
	m_resolutions.push_back("1280x720");

	m_onProcessEventHandle = Engine::get().processEventDelegate.bind(this, &GameWindow::onProcessEvent);
}

void GameWindow::onProcessEvent(SDL_Event& evt)
{
	if (!m_game)
	{
		return;
	}

	MLGE_SET_CURRENT_GAME_INSTANCE(m_game);

	// Simulate a mouse motion event.
	// This takes into account where the game is being rendered, so the event the game receives looks like it's running
	// in actual game mode and not in the editor
	if (evt.type == SDL_MOUSEMOTION)
	{
		if (m_hovered)
		{
			Game::MouseMotionEvent gameEvt;

			gameEvt.pos = {evt.motion.x, evt.motion.y};
			gameEvt.pos -= m_imGuiWindowPos;
			//CZ_LOG("Pos {},{}")
			gameEvt.rel = {evt.motion.xrel, evt.motion.yrel};

			Size renderTargetSize = m_game->getRenderTarget().getSize();
			gameEvt.pos.x = cz::clip(gameEvt.pos.x, 0, renderTargetSize.w - 1);
			gameEvt.pos.y = cz::clip(gameEvt.pos.y, 0, renderTargetSize.h - 1);

			m_game->onMouseMotion(gameEvt);
		}
	}
}

void GameWindow::showResolution(const char* resolutionStr)
{
	auto getSize = [](const std::string& str)
	{
		return Size{
			atoi(std::string(str, 0, str.find('x')).c_str()),
			atoi(std::string(str, str.find('x')+1).c_str())};
	};

	// Figure out if the game's resolution is one of the entries in the list
	int resolutionIdx = -1;
	for(size_t n = 0; n < m_resolutions.size(); n++)
	{
		if (m_resolutions[n] == resolutionStr)
		{
			resolutionIdx = static_cast<int>(n);
			break;
		}
	}

	if (ImGui::BeginCombo("Resolution", resolutionIdx == -1 ? nullptr : resolutionStr, ImGuiComboFlags_WidthFitPreview))
	{
		for (size_t n = 0; n < m_resolutions.size(); n++)
		{
			const bool isSelected = (resolutionIdx == static_cast<int>(n));
			if (ImGui::Selectable(m_resolutions[n].c_str(), isSelected))
			{
				Size newResolution = getSize(m_resolutions[n]);

				RenderTarget& renderTarget = m_game->getRenderTarget();
				if (renderTarget.getSize() != newResolution)
				{
					m_game->onWindowResized(newResolution);
				}
			}
		}

		ImGui::EndCombo();
	}

}

void GameWindow::show()
{
	// If this window is alive, then we must have a game running
	CZ_CHECK(m_game);

	MLGE_SET_CURRENT_GAME_INSTANCE(m_game);

	RenderTarget& renderTarget = m_game->getRenderTarget();

	int resX = renderTarget.getWidth();
	int resY = renderTarget.getHeight();

	std::string wallTime;
	std::string gameTime;

	auto getTimestamp = [](double timeSecs) -> std::string
	{
		std::chrono::milliseconds nowMs((int64_t)(timeSecs * 1000.0f));
		std::chrono::seconds nowSecs((int64_t)timeSecs);
		auto ms = nowMs - nowSecs;
		return std::format("{:%H:%M:%S}:{:03d}", nowSecs, ms.count());
	};

	char resStr[16];
	sprintf(resStr, "%dx%d", resX, resY);

	char buf[256];
	sprintf(
		buf,
		"Game %u - (%s) %s , WallTime: %s, GameTime: %s ###GameWindow_%u",
		m_id,
		m_game->hasFocus() ? "FOCUS" : "NO FOCUS",
		resStr,
		getTimestamp(m_game->getWallTimeSecs()).c_str(),
		getTimestamp(m_game->getGameTimeSecs()).c_str(),
		m_id);

	ImGuiWindowFlags noInputs = m_game->hasFocus() ? ImGuiWindowFlags_NoInputs : 0;
	ImGuiWindowFlags defaultFlags =  ImGuiWindowFlags_NoCollapse;
	if (!m_resizable)
	{
		defaultFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
	}
	
	if (m_game->hasFocus())
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	}

	// The ImGui game window
	ImGui::Begin(buf, nullptr, defaultFlags | noInputs);
	{
		ImGui::Checkbox("Resizable", &m_resizable);
		ImGui::SameLine();
		showResolution(resStr);


		//
		// The inner Button that actually shows the game and it's used to detect when we click the game to gain focus
		//
		if (SDL_Texture* tex = renderTarget.getTexture())
		{
			ImVec2 size;
			if (m_resizable)
			{
				size  = ImGui::GetContentRegionAvail();
				size.x -= ImGui::GetCursorPosX();
				size.y -= ImGui::GetCursorPosY();
				double now = m_game->getWallTimeSecs();

				if (Size::fromFloat(size.x, size.y) == m_resizeCountdown.newSize)
				{
					if (m_resizeCountdown.applyTime <= now && renderTarget.getSize() != m_resizeCountdown.newSize)
					{
						Size renderTargetSize = Size::fromFloat(size.x, size.y);
						m_game->onWindowResized(renderTargetSize);
					}
				}
				else
				{
					m_resizeCountdown.newSize = Size::fromFloat(size.x, size.y);
					m_resizeCountdown.applyTime = now + 1.0f;
				}


			}
			else
			{
				size = {(float)resX, (float)resY};
			}

			{
				ImVec2 tmp = ImGui::GetCursorScreenPos();
				m_imGuiWindowPos = {static_cast<int>(tmp.x), static_cast<int>(tmp.y)};
			}

			if (ImGui::ImageButton(tex, size, ImVec2(0,0), ImVec2(1,1), 0))
			{
				CZ_LOG(Log, "Switching focus to game window");
				Editor::get().setGameFocus(m_game, true);
			}

			// If we gave focus to the game, then by definition we are hovering, therefore don't process this because it will
			// end up sending "window leave" events to the game by mistake
			if (!m_game->hasFocus())
			{
				bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);
				// Simulate the Window Enter/Leave event while in Editor mode
				if (hovered != m_hovered)
				{
					m_hovered = hovered;
					m_game->onWindowEnter(m_hovered);
				}

				if (m_hovered)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_None);
				}
			}
		}
	}

	ImGui::End();
}

bool GameWindow::tick(float /*elapsedSeconds*/)
{
	MLGE_PROFILE_SCOPE(mlge_GameWindow_tick);

	show();
	return true;
}

} // namespace mlge::editor


#endif

