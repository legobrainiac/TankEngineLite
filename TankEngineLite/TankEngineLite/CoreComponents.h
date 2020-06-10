#ifndef CORE_COMPONENTS_H
#define CORE_COMPONENTS_H

#include "ecs.h"

#include <functional>
#include <DirectXMath.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

#include "InputManager.h"

class Model;
class Effect;
class Texture;
class SpriteBatch;

using namespace DirectX;

class Utils
{
public:
	template<typename T>
	static constexpr T Lerp(T start, T end, float percent)
	{
		return (start + percent * (end - start));
	}

	static int RandInterval(int min, int max)
	{
		return min + rand() % (max + 1 - min);
	}
};


// TODO(tomas): Document both transform and camera components

//////////////////////////////////////////////////////////////////////////
// Component: TransformComponent
// Description: Simple transform with a position, rotation and scale
class TransformComponent
	: public ECS::EntityComponent
{
public:
	TransformComponent()
		: position({ 0.f, 0.f, 0.f })
		, scale({ 1.f, 1.f, 1.f })
	{
		XMStoreFloat4(&rotationQ, XMQuaternionIdentity());
	}

	TransformComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
		, position({ 0.f, 0.f, 0.f })
		, scale({ 1.f, 1.f, 1.f })
	{ 
		XMStoreFloat4(&rotationQ, XMQuaternionIdentity());
	}

	~TransformComponent() { }

	XMFLOAT4X4 GetWorldMatrix() const
	{
		XMFLOAT4X4 worldMatrix{};
		
		auto rMat = XMMatrixRotationQuaternion(XMLoadFloat4(&rotationQ));
		auto tMat = XMMatrixTranslation(position.x, position.y, position.z);
		auto sMat = XMMatrixScaling(scale.x, scale.y, scale.z);

		XMStoreFloat4x4(&worldMatrix, sMat * (rMat * tMat));
		return worldMatrix;
	}

	void Translate(const XMFLOAT3& translation) 
	{ 
		XMStoreFloat3(&position, XMLoadFloat3(&translation) + XMLoadFloat3(&position));
	}

	void Rotate(float yaw, float pitch, float roll)
	{
		XMVECTOR rawRot = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
		XMStoreFloat4(&rotationQ, XMQuaternionMultiply(XMLoadFloat4(&rotationQ), rawRot));
	}

public:
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotationQ;
};

class CameraComponent
	: public ECS::EntityComponent
{
public:
	CameraComponent()
		: m_pEntityTransform(nullptr)
		, m_ViewMatrix()
		, m_MeetsRequirements(false)
	{
	}

	CameraComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
		, m_ViewMatrix()
		, m_MeetsRequirements(false)
	{
		m_pEntityTransform = pE->GetComponent<TransformComponent>();

		if (m_pEntityTransform != nullptr)
		{
			m_MeetsRequirements = true;
			GenerateViewMatrix();
		}
	}

	void Update([[maybe_unused]] float dt) override
	{
		auto pInput = InputManager::GetInstance();

		dt *= 10.f;

		if (pInput->IsKeyDown(SDL_SCANCODE_W))
			m_pEntityTransform->Translate({0.f, 0.f, -1.f * dt});

		if (pInput->IsKeyDown(SDL_SCANCODE_S))
			m_pEntityTransform->Translate({0.f, 0.f, 1.f * dt});

		if (pInput->IsKeyDown(SDL_SCANCODE_A))
			m_pEntityTransform->Translate({1.f * dt, 0.f, 0.f}); 

		if (pInput->IsKeyDown(SDL_SCANCODE_D))
			m_pEntityTransform->Translate({-1.f * dt, 0.f, 0.f}); 

		if (pInput->IsKeyDown(SDL_SCANCODE_E))
			m_pEntityTransform->Translate({0.f, -1.f * dt, 0.f}); 

		if (pInput->IsKeyDown(SDL_SCANCODE_Q))
			m_pEntityTransform->Translate({0.f, 1.f * dt, 0.f}); 

		GenerateViewMatrix();
	}

	void GenerateViewMatrix();

	constexpr XMFLOAT4X4& GetViewMatrix() noexcept { return m_ViewMatrix; }
	constexpr XMFLOAT3& GetPosition() const { return m_pEntityTransform->position; }

private:
	TransformComponent* m_pEntityTransform;
	XMFLOAT4X4 m_ViewMatrix;
	bool m_MeetsRequirements;
};

//////////////////////////////////////////////////////////////////////////
// Component: TransformComponent2D
// Description: Simple 2D transform with a position, rotation and scale
class TransformComponent2D
	: public ECS::EntityComponent
{
public:
	TransformComponent2D() 
		: position({ 0.f, 0.f, 0.f })
		, scale({ 1.f, 1.f })
		, rotation(0.f)
	{
	}

	TransformComponent2D(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
		, position({ 0.f, 0.f, 0.f })
		, scale({ 1.f, 1.f })
		, rotation(0.f)
	{
	}

	// These are made public for ease of access and manipulation
	XMFLOAT3 position;
	XMFLOAT2 scale;
	float rotation;
};

// Render Component
using CustomRenderFunction = std::function<void(ECS::Entity*)>;

//////////////////////////////////////////////////////////////////////////
// Component: SpriteRenderComponent
// Description: Fairly basic render component, pushes a sprite on to a sprite batch every frame.
//	Also allows you to inject rendering code in to the sprite batch loop with a Custom Render Function
class SpriteRenderComponent
	: public ECS::EntityComponent
{
public:
	SpriteRenderComponent()
		: m_pTransform(nullptr)
		, m_pSpriteBatch(nullptr)
		, m_AtlasTransform()
		, m_MeetsRequirements(false)
		, m_CustomRenderFunction()
		, m_bShouldCustomRender(false)
	{
	}

	SpriteRenderComponent(ECS::Entity* pE);

	//////////////////////////////////////////////////////////////////////////
	// Method:    SetSpriteBatch
	// FullName:  SpriteRenderComponent::SetSpriteBatch
	// Access:    public 
	// Returns:   void
	// Description: Set the SpriteBatch this renderer should use
	// Parameter: SpriteBatch * pBatch
	inline void SetSpriteBatch(SpriteBatch* pBatch) { m_pSpriteBatch = pBatch; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetSpriteBatch
	// FullName:  SpriteRenderComponent::GetSpriteBatch
	// Access:    public 
	// Returns:   SpriteBatch*
	// Qualifier: const noexcept
	// Description: Get the SpriteBatch in use
	inline SpriteBatch* GetSpriteBatch() const noexcept { return m_pSpriteBatch; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    SetAtlasTransform
	// FullName:  SpriteRenderComponent::SetAtlasTransform
	// Access:    public 
	// Returns:   void
	// Qualifier: With in the SpriteBatch texture atlas, what transform should it use
	// Parameter: const XMFLOAT4 & transform
	inline void SetAtlasTransform(const XMFLOAT4& transform) { m_AtlasTransform = transform; }
	
	//////////////////////////////////////////////////////////////////////////
	// Method:    Render
	// FullName:  SpriteRenderComponent::Render
	// Access:    public 
	// Returns:   void
	// Qualifier: const
	// Description: Render the sprite batch
	void Render() const;

	//////////////////////////////////////////////////////////////////////////
	// Method:    SetCustomRenderFunction
	// FullName:  SpriteRenderComponent::SetCustomRenderFunction
	// Access:    public 
	// Returns:   void
	// Description: Can be used to inject custom actions in to the rendering loop
	// Parameter: const CustomRenderFunction & crf
	inline void SetCustomRenderFunction(const CustomRenderFunction& crf);

	//////////////////////////////////////////////////////////////////////////
	// Method:    SetPivot
	// FullName:  SpriteRenderComponent::SetPivot
	// Access:    public 
	// Returns:   void
	// Description: Set Rendering Pivot
	// Parameter: const XMFLOAT2& piv
	inline void SetPivot(const XMFLOAT2& piv) { m_Pivot = piv; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    ResetToDefault
	// FullName:  SpriteRenderComponent::ResetToDefault
	// Access:    public 
	// Returns:   void
	// Description: SpriteRenderComponent::m_bShouldCustomRender = false;
	inline void ResetToDefault() { m_bShouldCustomRender = false; }

private:
	TransformComponent2D* m_pTransform;
	SpriteBatch* m_pSpriteBatch;
	XMFLOAT4 m_AtlasTransform;
	XMFLOAT2 m_Pivot{ 0.5f, 0.5f };
	bool m_MeetsRequirements;

	// Custom rendering
	CustomRenderFunction m_CustomRenderFunction;
	bool m_bShouldCustomRender;
};

//////////////////////////////////////////////////////////////////////////
// TODO(tomas): seperate file
class ModelRenderComponent
	: public ECS::EntityComponent
{
public:
	ModelRenderComponent()
	{
	}

	~ModelRenderComponent()
	{
		// TODO(tomas): tis wrong ples help
// 		m_pWorld->Release();
// 		m_pDiffuseMap->Release();
// 		m_pWorldViewProjection->Release();
// 		m_pInputLayout->Release();
// 		m_pTechnique->Release();
	}

	ModelRenderComponent(ECS::Entity* pE);

	void Initialize(Model* pModel, Texture* pTexture);
	void Render();

private:
	Model* m_pModel;
	Texture* m_pTexture;
	Effect* m_pEffect;

	// TODO(tomas): abstraction in to materials
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMap;
	ID3DX11EffectMatrixVariable* m_pWorldViewProjection;
	ID3DX11EffectMatrixVariable* m_pWorld;

	TransformComponent* m_pTransform;
	bool m_MeetsRequirements;
};

#endif

