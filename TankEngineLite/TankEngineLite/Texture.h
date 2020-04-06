#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>
#include <string>

#include "D3D.h"

class Texture
{
public:
	Texture();
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	ID3D11ShaderResourceView* GetTexture() const { return m_pTextureView; };
	bool Initialize(ID3D11Device* pDevice, LPCSTR texName);
	void Shutdown();

	inline const XMFLOAT2& GetTextureSize() { return m_Size; }

private:
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureView;

	XMFLOAT2 m_Size;
};

#endif // !TEXTURE_H