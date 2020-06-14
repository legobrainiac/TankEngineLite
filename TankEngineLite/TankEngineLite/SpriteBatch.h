#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include <vector>

#include "D3D.h"
#include "Texture.h"

//////////////////////////////////////////////////////////////////////////
// Enum: BatchMode
// Description: Describes in which way the batch rendering is handled, 
//		particularly the discard policy
// Enum options:
//		BATCHMODE_DYNAMIC,
//		BATCHMODE_STATIC
enum BatchMode
{
	BATCHMODE_DYNAMIC,
	BATCHMODE_STATIC,
};

//////////////////////////////////////////////////////////////////////////
// Struct: Batch item
// Description: This is the representation of an item on the batch both here and
//		in the GPU
struct BatchItem
{
	XMFLOAT4 atlasTrasform;
	XMFLOAT4 transformData0;
	XMFLOAT4 transformData1;
	XMFLOAT4 color;
};

//////////////////////////////////////////////////////////////////////////
// Class: SpriteBatch
// Description: SpriteBatch is built to reduced draw calls when doing 2D rendering
// Constructor: SpriteBatch::SpriteBatch(const std::string_view& name)
// Public Methods:
//		SpriteBatch::InitializeBatch(...)
//		SpriteBatch::Destroy()
//		SpriteBatch::PushSprite(...)
//		SpriteBatch::Render()
//		SpriteBatch::GetTextureView()
//		SpriteBatch::GetName()
class SpriteBatch
{
public:
	SpriteBatch(const std::string_view& name)
		: m_Name(name)
		, m_Batch()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// If a sprite batch is static, the buffer will not be refreshed every frame
	//  you'll need to restart the batch to create a new buffer (IE: static world)
	void InitializeBatch(Texture* atlas, BatchMode mode = BatchMode::BATCHMODE_DYNAMIC);
	void Destroy();

	//////////////////////////////////////////////////////////////////////////
	// Method:    PushSprite
	// FullName:  SpriteBatch::PushSprite
	// Access:    public 
	// Returns:   void
	// Description: Used to push sprites on to current batch, used for both dynamic and static initialization
	// Parameter: XMFLOAT4 atlasTransform
	// Parameter: XMFLOAT3 position
	// Parameter: float rotation
	// Parameter: XMFLOAT2 scale
	// Parameter: XMFLOAT2 pivot
	// Parameter: XMFLOAT4 colour
	void PushSprite(XMFLOAT4 atlasTransform, XMFLOAT3 position, float rotation, XMFLOAT2 scale, XMFLOAT2 pivot, XMFLOAT4 colour);

	//////////////////////////////////////////////////////////////////////////
	// Method:    Render
	// FullName:  SpriteBatch::Render
	// Access:    public 
	// Returns:   void
	// Description: Update the buffer on the GPU(if necessary) and render the batch
	void Render();

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetTextureView
	// FullName:  SpriteBatch::GetTextureView
	// Access:    public 
	// Returns:   ID3D11ShaderResourceView*
	// Qualifier: const
	inline ID3D11ShaderResourceView* GetTextureView() const { return m_Atlas->GetTexture(); }

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetName
	// FullName:  SpriteBatch::GetName
	// Access:    public 
	// Returns:   constexpr std::string_view&
	// Qualifier:
	constexpr std::string_view& GetName() { return m_Name; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    SetIsRendering
	// FullName:  SpriteBatch::SetIsRendering
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: bool val
	void SetIsRendering(bool val) { m_IsRendering = val; }

	constexpr void SetCamera(XMFLOAT2 camPos) noexcept { m_CameraPos = camPos; }
	constexpr void SetScale(float scale) noexcept { m_Scale = scale; }

	// Don't worry about this one, only here for debugging purposes
	void ImGuiDebug();

private:
	//////////////////////////////////////////////////////////////////////////
	// Update buffer with the freshest vertices straight from the game programmer
	void UpdateBuffer();

private:
	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;
	ID3DX11EffectMatrixVariable* m_pTransfromMatrixV = nullptr;
	ID3DX11EffectVectorVariable* m_pTextureSizeV = nullptr;
	ID3DX11EffectVectorVariable* m_pCameraPos = nullptr;
	ID3DX11EffectScalarVariable* m_pScale = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pTextureSRV = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	XMFLOAT4X4 m_Transform{};
	
	XMFLOAT2 m_CameraPos{};
	float m_Scale = 1.f;

	Texture* m_Atlas = nullptr;
	std::string_view m_Name{};
	std::vector<BatchItem> m_Batch{};
	int m_BatchSize = 0;
	bool m_Dirty = false;
	bool m_IsRendering = true;
	BatchMode m_Mode = BATCHMODE_DYNAMIC;
};

#endif // !SPRITE_BATCH_H
