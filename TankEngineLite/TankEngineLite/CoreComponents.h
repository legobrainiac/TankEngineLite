#ifndef CORE_COMPONENTS_H
#define CORE_COMPONENTS_H

#include "ecs.h"

#include <functional>
#include <DirectXMath.h>

class SpriteBatch;
using namespace DirectX;

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
	bool m_MeetsRequirements;

	// Custom rendering
	CustomRenderFunction m_CustomRenderFunction;
	bool m_bShouldCustomRender;
};

#endif

