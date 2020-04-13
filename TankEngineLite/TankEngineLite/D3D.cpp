#include "D3D.h"
#include "D3D.h"

bool D3D::Initialize(int screenW, int screenH, bool vsync, HWND hwnd)
{
	m_ScreenW = screenW;
	m_ScreenH = screenH;
	m_Vsync = vsync;

	// Initialize dx11
	HRESULT result;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   // Create device and device context
	DXCALL(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext));

	// Create DXGIFactory
	DXCALL(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory)));

	// Swap chain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = screenW;
	swapChainDesc.BufferDesc.Height = screenH;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 144;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = hwnd;

	// Create the swap chain and hook it into the handle of the SDL window
	DXCALL(m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain));

	// Debug layer
#if defined(DEBUG) || defined(_DEBUG)
	DXCALL(m_pDevice->QueryInterface(&m_pDebug));
#endif

	// Depth/Stencil buffer and view
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = screenW;
	depthStencilDesc.Height = screenH;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilViewDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create depth stencil texture and view
	DXCALL(m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer));
	DXCALL(m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView));
	
	// Create render target view
	DXCALL(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer)));
	DXCALL(m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView));
	
	// Bind the views to the output merger stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Set the viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_ScreenW);
	viewport.Height = static_cast<float>(m_ScreenH);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	m_pDeviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix
	float fov, aspectRatio;
	fov = static_cast<float>(XM_PI) / 2.f;
	aspectRatio = static_cast<float>(m_ScreenW) / static_cast<float>(m_ScreenH);

	// Create the projection matrix for 3D Rendering
	XMStoreFloat4x4(&m_ProjectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, 0.1f, 1000.f));

	// Create world matrix
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	// Create orthographic matrix
	XMStoreFloat4x4(&m_OrthoMatrix, XMMatrixOrthographicLH((float)m_ScreenW, (float)m_ScreenH, 0.1f, 1000.f));

	DXRELEASE(m_pDXGIFactory);

	return true;
}

void D3D::Shutdown()
{
	// Depth stencil view and render target view
	DXRELEASE(m_pDepthStencilView);
	DXRELEASE(m_pDepthStencilBuffer);

	DXRELEASE(m_pRenderTargetView);
	DXRELEASE(m_pRenderTargetBuffer);

	// Swap chain
	DXRELEASE(m_pSwapChain);
	
	// Device
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		DXRELEASE(m_pDeviceContext);
	}

	DXRELEASE(m_pDevice);

	// Look for live objects
#if defined(DEBUG) || defined(_DEBUG)
	if (m_pDebug)
	{
		//m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		DXRELEASE(m_pDebug);
	}
#endif
}

void D3D::Begin(const DirectX::XMFLOAT4& colour)
{
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &colour.x);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_BIND_DEPTH_STENCIL, 1.f, 0);
}

void D3D::End()
{
	m_pSwapChain->Present((m_Vsync) ? 1 : 0, 0);
}

std::tuple<XMFLOAT4X4, XMFLOAT4X4, XMFLOAT4X4> D3D::GetMatrices() const
{
	return std::tuple<XMFLOAT4X4, XMFLOAT4X4, XMFLOAT4X4>();
}
