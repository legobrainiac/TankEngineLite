#include "pch.h"
#include "Renderer.h"

#include "Texture.h"
#include "BasicComponents.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl.h"

#include "D3D.h"

#include <SDL.h>

bool Renderer::Init(SDL_Window* pWin, int xw, int yh)
{
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
		bool result = m_pDirectX->Initialize(xw, yh, true, hwnd);

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
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	 
	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForD3D(pWin);
	ImGui_ImplDX11_Init(m_pDirectX->GetDevice(), m_pDirectX->GetDeviceContext());

	return true;
}

void Renderer::Render(ECS::System* pWorldRenderSystem) const
{
	if (pWorldRenderSystem)
	{
		// Render world
		pWorldRenderSystem->ForAll([](ECS::EntityComponent* pC)
			{
				static_cast<RenderComponent*>(pC)->Render();
			});
	}
}

// Engine related rendering and ImGui
void Renderer::RootRenderBegin() const
{
	ImGui::Render();
	m_pDirectX->Begin({ 0.392f, 0.584f, 0.929f, 1.f });
}

void Renderer::RootRenderEnd() const
{
	// Render imgui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers and present the rendered scene  
	m_pDirectX->End();
}

void Renderer::Destroy()
{
	DXSHUTDOWN(m_pDirectX);
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::RenderTexture(Texture* pTexture, const float x, const float y) const
{
	_(pTexture);
	SDL_Rect dst;
	dst.x = static_cast<int>(x);
	dst.y = static_cast<int>(y);
	//SDL_QueryTexture(pTexture->GetSDLTexture(), nullptr, nullptr, &dst.w, &dst.h);
	//SDL_RenderCopy(GetSDLRenderer(), pTexture->GetSDLTexture(), nullptr, &dst);
}

void Renderer::RenderTexture(Texture* pTexture, const float x, const float y, const float width, const float height) const
{
	_(pTexture);
	SDL_Rect dst;
	dst.x = static_cast<int>(x);
	dst.y = static_cast<int>(y);
	dst.w = static_cast<int>(width);
	dst.h = static_cast<int>(height);
	//SDL_RenderCopy(GetSDLRenderer(), pTexture->GetSDLTexture(), nullptr, &dst);
}
