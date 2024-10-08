#if MLGE_EDITOR

#include "mlge/Editor/GameWindow.h"
#include "mlge/Editor/Editor.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Profiler.h"
#include "mlge/Engine.h"
#include "crazygaze/core/Algorithm.h"

namespace mlge::editor
{

GameWindow::GameWindow()
{
	m_onProcessEventHandle = Engine::get().processEventDelegate.bind(this, &GameWindow::onProcessEvent);
}

void GameWindow::onProcessEvent(SDL_Event& evt)
{
	if (!Game::tryGet())
	{
		return;
	}

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

			Size renderTargetSize = Game::get().getRenderTarget().getSize();
			gameEvt.pos.x = cz::clip(gameEvt.pos.x, 0, renderTargetSize.w - 1);
			gameEvt.pos.y = cz::clip(gameEvt.pos.y, 0, renderTargetSize.h - 1);

			Game::get().onMouseMotion(gameEvt);
		}
	}
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

			{
				ImVec2 tmp = ImGui::GetCursorScreenPos();
				m_imGuiWindowPos = {static_cast<int>(tmp.x), static_cast<int>(tmp.y)};
			}

			if (ImGui::ImageButton(tex, size, ImVec2(0,0), ImVec2(1,1), 0))
			{
				CZ_LOG(Log, "Switching focus to game window");
				Editor::get().setGameFocus(true);
			}

			// If we gave focus to the game, then by definition we are hovering, therefore don't process this because it will
			// end up sending "window leave" events to the game by mistake
			if (!Game::get().hasFocus())
			{
				bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);
				// Simulate the Window Enter/Leave event while in Editor mode
				if (hovered != m_hovered)
				{
					m_hovered = hovered;
					Game::get().onWindowEnter(m_hovered);
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

