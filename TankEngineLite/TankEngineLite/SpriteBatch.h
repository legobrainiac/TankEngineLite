#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <vector>
#include "D3D.h"

class Texture;

//////////////////////////////////////////////////////////////////////////
// Sprite batch renderer based on the one we did in Graphics Programming 2

inline bool XMFloat4Equals(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

struct BatchItem
{
	XMFLOAT4 atlasTrasform;
	XMFLOAT4 transformData0;
	XMFLOAT4 transformData1;
	XMFLOAT4 color;

	bool operator==(const BatchItem& rhs)
	{
		if (!XMFloat4Equals(rhs.transformData0, transformData0)) return false;
		if (!XMFloat4Equals(rhs.transformData1, transformData1)) return false;
		if (!XMFloat4Equals(rhs.color, color)) return false;

		return true;
	}
};

//////////////////////////////////////////////////////////////////////////
// The sprite batch aims to help bring down the number of draw calls
// USE:
//	Some init function:
//		auto pSpriteBatch = new SpriteBatch();
//		pSpriteBatch->InitializeBatch(vectorOfTexturePointers);
//	
//	Some render function:
//		m_pRenderComponentSystem([pSpriteBatch](RenderComponent* pComp)
//		{
//			pSpriteBatch->PushSprite(ATLAS_ID, ...);
//		});
class SpriteBatch
{
public:
	//////////////////////////////////////////////////////////////////////////
	// If a sprite batch is static, the buffer will not be refreshed every frame
	//  you'll need to restart the batch to create a new buffer (IE: static world)
	void InitializeBatch(Texture* atlas, bool isStatic = false);
	void Destroy();

	//////////////////////////////////////////////////////////////////////////
	// This will be used both for static and dynamic batch initialization
	void PushSprite(XMFLOAT4 atlasTransform, XMFLOAT3 position, float rotation, XMFLOAT2 scale, XMFLOAT2 pivot, XMFLOAT4 colour);

	//////////////////////////////////////////////////////////////////////////
	// Render the actual sprite batch
	void Render();

private:
	//////////////////////////////////////////////////////////////////////////
	// Update buffer with the freshest vertices straight from the game programmer
	void UpdateBuffer();

private:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;
	ID3DX11EffectMatrixVariable* m_pTransfromMatrixV;
	ID3DX11EffectVectorVariable* m_pTextureSizeV;
	ID3DX11EffectShaderResourceVariable* m_pTextureSRV;
	ID3D11Buffer* m_pVertexBuffer;
	XMFLOAT4X4 m_Transform;

	Texture* m_Atlas;
	std::vector<BatchItem> m_Batch;
	int m_BatchSize;
	bool m_Dirty;
};

#endif // !SPRITE_BATCH_H
