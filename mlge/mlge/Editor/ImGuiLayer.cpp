
#if MLGE_EDITOR

#include "mlge/Editor/ImGuiLayer.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Editor/Editor.h"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"


namespace mlge::editor
{

void ImGuiLayer::init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;

	// Setup style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(Renderer::get().getSDLWindow(), Renderer::get().getSDLRenderer());
	ImGui_ImplSDLRenderer2_Init(Renderer::get().getSDLRenderer());

	m_initialized = true;
}

void ImGuiLayer::shutdown()
{
	if (m_initialized)
	{
		ImGui_ImplSDLRenderer2_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
		m_initialized = false;
	}
}

void ImGuiLayer::processEvent(SDL_Event& evt)
{
	if (Editor::get().gameHasFocus())
	{
		return;
	}

	ImGui_ImplSDL2_ProcessEvent(&evt);

	ImGuiIO& io = ImGui::GetIO();

	if (io.WantCaptureMouse != m_wantCaptureMouse || m_firstFrame)
	{
		m_wantCaptureMouse = io.WantCaptureMouse;
		CZ_LOG(VeryVerbose, "ImGui::WantCaptureMouse = {}", m_wantCaptureMouse);
	}

	if (io.WantCaptureKeyboard != m_wantCaptureKeyboard || m_firstFrame)
	{
		m_wantCaptureKeyboard = io.WantCaptureKeyboard;
		CZ_LOG(VeryVerbose, "ImGui::WantCaptureKeyboard = {}", m_wantCaptureKeyboard);
	}

	m_firstFrame = false;
}

void ImGuiLayer::beginFrame()
{
	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	if (Editor::get().gameHasFocus())
	{
		ImGui::GetIO().ClearInputKeys();
	}
}

void ImGuiLayer::endFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	SDL_RenderSetScale(Renderer::get().getSDLRenderer(), io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
}

uint32_t ImGuiLayer::getActiveWidgetId() const
{
	return GImGui->ActiveId;
}

} // namespace mlge::editor

#endif

