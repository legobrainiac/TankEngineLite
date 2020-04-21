#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>
#include <string>

#include "D3D.h"

//////////////////////////////////////////////////////////////////////////
// Class: Texture
// Description: Wrapper around the DirectX representations of a Texture and its resource view
class Texture
{
public:
	Texture();
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	//////////////////////////////////////////////////////////////////////////
	// Method:    Initialize
	// FullName:  Texture::Initialize
	// Access:    public 
	// Returns:   bool
	// Description: Initialize texture from the given file path
	// Parameter: ID3D11Device* pDevice
	// Parameter: LPCSTR texName
	bool Initialize(ID3D11Device* pDevice, LPCSTR texName);

	//////////////////////////////////////////////////////////////////////////
	// Method:    Shutdown
	// FullName:  Texture::Shutdown
	// Access:    public 
	// Returns:   void
	// Description: Shutdown and release all texture related resources
	void Shutdown();

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetTexture
	// FullName:  Texture::GetTexture
	// Access:    public 
	// Returns:   constexpr ID3D11ShaderResourceView*
	// Qualifier: noexcept
	constexpr ID3D11ShaderResourceView* GetTexture() noexcept { return m_pTextureView; };

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetTextureSize
	// FullName:  Texture::GetTextureSize
	// Access:    public 
	// Returns:   constexpr DirectX::XMFLOAT2&
	// Qualifier: noexcept
	constexpr XMFLOAT2& GetTextureSize() noexcept { return m_Size; }

private:
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureView;

	XMFLOAT2 m_Size;
};

#endif // !TEXTURE_H