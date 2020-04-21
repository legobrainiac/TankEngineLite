#ifndef D3D_H
#define D3D_H

#include <dxgi.h>
#include <d3d11.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#include <tuple>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl.h"

#define DXASSERT() if (FAILED(result)) return false;
#define DXCALL(CALL) result = CALL; DXASSERT();
#define DXSHUTDOWN(RES) if(RES) { RES->Shutdown(); delete RES; RES = nullptr; }
#define DXRELEASE(P_OBJ)do { if(P_OBJ) { P_OBJ->Release(); P_OBJ = nullptr; } } while(0);

using namespace DirectX;

class D3D
{
public:
	D3D() {}
	~D3D()
	{
		Shutdown();
	}

	bool Initialize(int screenW, int screenH, bool vsync, HWND hwnd);
	void Shutdown();

	void Begin(const DirectX::XMFLOAT4& colour);
	void End();

	constexpr ID3D11Device* GetDevice() const { return m_pDevice; };
	constexpr ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext; };

	constexpr XMFLOAT4X4 GetProjectionMatrix() noexcept { return m_ProjectionMatrix; }
	constexpr XMFLOAT4X4 GetWorldMatrix() noexcept { return m_WorldMatrix; }
	constexpr XMFLOAT4X4 GetOrthoMatrix() noexcept { return m_OrthoMatrix; }

	std::tuple<XMFLOAT4X4, XMFLOAT4X4, XMFLOAT4X4> GetMatrices() const;

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	IDXGIFactory* m_pDXGIFactory;

	ID3D11Debug* m_pDebug;

	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;

	ID3D11Resource* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;

	int m_ScreenW;
	int m_ScreenH;
	bool m_Vsync;

	// Matrices
	XMFLOAT4X4 m_ProjectionMatrix;
	XMFLOAT4X4 m_WorldMatrix;
	XMFLOAT4X4 m_OrthoMatrix;
};

#endif // !D3D_H

