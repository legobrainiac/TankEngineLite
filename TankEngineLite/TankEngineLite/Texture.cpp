#include "Texture.h"
#include "SDL_image.h"
#include "Renderer.h"

Texture::Texture()
	: m_pTextureView(nullptr)
	, m_pTexture(nullptr)
{
}

bool Texture::Initialize(LPCSTR fileName)
{
	const auto pDevice = Renderer::GetInstance()->GetDirectX()->GetDevice();
	std::string f = fileName;
	std::wstring wfilename = std::wstring(f.begin(), f.end());

	auto pSurface = IMG_Load(f.c_str());
	if (!pSurface)
		return false;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	m_Size = { float(pSurface->w), float(pSurface->h) };

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);
	DXASSERT();
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTextureView);
	DXASSERT();

	SDL_FreeSurface(pSurface);
	return true;
}

void Texture::Shutdown()
{
	DXRELEASE(m_pTextureView);
	DXRELEASE(m_pTexture);
}
