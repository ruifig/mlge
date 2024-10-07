#if MLGE_EDITOR

#include "mlge/Editor/GameWindow.h"
#include "mlge/Editor/Editor.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Profiler.h"

namespace mlge::editor
{

GameWindow::GameWindow()
{
}

void GameWindow::show()
{
	// If this window is alive, then we must have a game running
	CZ_CHECK(Game::tryGet() && Editor::get().gameIsRunning());

	RenderTarget& renderTarget = Game::get().getRenderTarget();

	int resX = renderTarget.getWidth();
	int resY = renderTarget.getHeight();

	bool gameHasFocus = Editor::get().gameHasFocus();

	std::string wallTime;
	std::string gameTime;

	auto getTimestamp = [](double timeSecs) -> std::string
	{
		std::chrono::milliseconds nowMs((int64_t)(timeSecs * 1000.0f));
		std::chrono::seconds nowSecs((int64_t)timeSecs);
		auto ms = nowMs - nowSecs;
		return std::format("{:%H:%M:%S}:{:03d}", nowSecs, ms.count());
	};

	char buf[256];
	sprintf(
		buf,
		"GameWindow (%s) %dpx * %dpx, WallTime: %s, GameTime: %s ###GameWindow",
		gameHasFocus ? "FOCUS" : "NO FOCUS",
		resX, resY,
		getTimestamp(Game::get().getWallTimeSecs()).c_str(),
		getTimestamp(Game::get().getGameTimeSecs()).c_str());


	ImGuiWindowFlags noInputs = gameHasFocus ? ImGuiWindowFlags_NoInputs : 0;
	ImGuiWindowFlags defaultFlags =  ImGuiWindowFlags_NoCollapse;
	if (!m_resizable)
	{
		defaultFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
	}
	
	if (gameHasFocus)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	}

	// The ImGui game window
	ImGui::Begin(buf, nullptr, defaultFlags | noInputs);
	{
		ImGui::Checkbox("Resizable", &m_resizable);

		// The inner Button that actually shows the game and it's used to detect when we press the game to gain focus
		if (SDL_Texture* tex = renderTarget.getTexture())
		{
			ImVec2 size;
			if (m_resizable)
			{
				size  = ImGui::GetContentRegionAvail();
				size.x -= ImGui::GetCursorPosX();
				size.y -= ImGui::GetCursorPosY();
				double now = Game::get().getWallTimeSecs();

				if (Size::fromFloat(size.x, size.y) == m_resizeCountdown.newSize)
				{
					if (m_resizeCountdown.applyTime <= now && renderTarget.getSize() != m_resizeCountdown.newSize)
					{
						Size renderTargetSize = Size::fromFloat(size.x, size.y);
						Game::get().onWindowResized(renderTargetSize);
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

			if (ImGui::ImageButton(tex, size))
			{
				CZ_LOG(Log, "Switching focus to game window");
				Editor::get().setGameFocus(true);
			}

			// Simulate the Window Enter/Leave event while in Editor mode
			{
				bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);
				if (hovered != m_entered)
				{
					m_entered = hovered;
					Game::get().onWindowEnter(m_entered);
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

