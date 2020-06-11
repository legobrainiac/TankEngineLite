#include "pch.h"
#include "Renderer.h"

#include <SDL.h>

#include "D3D.h"
#include "CoreComponents.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl.h"
#include "Profiler.h"

bool Renderer::Init(SDL_Window* pWin, int xw, int yh, bool vSync)
{
	LOGGER->Log<LOG_INFO>("Initializing renderer");

	m_pDirectX = new D3D();

	m_Xw = xw;
	m_Yh = yh;

	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(pWin, &sysWMInfo);
	HWND hwnd = sysWMInfo.info.win.window;
	m_pWindow = pWin;

	if (m_pDirectX)
	{
		bool result = m_pDirectX->Initialize(xw, yh, vSync, hwnd);

		if (!result)
		{
			MessageBox(hwnd, "Could not initialize Direct3D", "Error", MB_OK);
			return false;
		}
	}
	else
		return false;

	//////////////////////////////////////////////////////////////////////////
	// ImGui setup

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	[[maybe_unused]] ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	 
	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForD3D(pWin);
	ImGui_ImplDX11_Init(m_pDirectX->GetDevice(), m_pDirectX->GetDeviceContext());

	return true;
}

void Renderer::ModelRender(ECS::System* pModelRenderSystem)
{
	// If the world system is valid
	if (pModelRenderSystem)
	{
		// Populate sprite batching using the sprite batch render components
		pModelRenderSystem->ForAll([](ECS::EntityComponent* pC)
			{
				static_cast<ModelRenderComponent*>(pC)->Render();
			});
	}
}

void Renderer::SpriteBatchRender(ECS::System* pWorldRenderSystem) const
{
	// If the world system is valid
	if (pWorldRenderSystem)
	{
		// Populate sprite batching using the sprite batch render components
		pWorldRenderSystem->ForAll([](ECS::EntityComponent* pC)
			{
				static_cast<SpriteRenderComponent*>(pC)->Render();
			});
	}
}

// Engine related rendering and ImGui
void Renderer::RootRenderBegin() const
{
	Profiler::GetInstance()->BeginSubSession<SESSION_RENDER_BEGIN>();
	ImGui::Render();
	m_pDirectX->Begin({ 0.f, 0.f, 0.f, 1.f });
	Profiler::GetInstance()->EndSubSession();
}

void Renderer::RootRenderEnd() const
{
	Profiler::GetInstance()->BeginSubSession<SESSION_RENDER_END>();
	// Render imgui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers and present the rendered scene  
	m_pDirectX->End();
	Profiler::GetInstance()->EndSubSession();
}

void Renderer::Destroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	DXSHUTDOWN(m_pDirectX);
}