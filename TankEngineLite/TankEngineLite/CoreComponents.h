#ifndef CORE_COMPONENTS_H
#define CORE_COMPONENTS_H

#include "ecs.h"

#include <functional>
#include <DirectXMath.h>

class SpriteBatch;
using namespace DirectX;

// Transform component
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

class SpriteRenderComponent
	: public ECS::EntityComponent
{
public:
	SpriteRenderComponent()
		: m_pTransform(nullptr)
		, m_pSpriteBatch(nullptr)
		, m_AtlasTransform()
		, m_MeetsRequirements(false)
	{
	}

	SpriteRenderComponent(ECS::Entity* pE);

	inline void SetSpriteBatch(SpriteBatch* pBatch) { m_pSpriteBatch = pBatch; }
	inline SpriteBatch* GetSpriteBatch() const { return m_pSpriteBatch; }

	inline void SetAtlasTransform(const XMFLOAT4& transform) { m_AtlasTransform = transform; }
	void Render() const;

	// Custom rendering function
	// Can also be used if you need to inject something in to the rendering loop
	inline void SetCustomRenderFunction(const CustomRenderFunction& crf);
	inline void ResetToDefault() { m_bShouldCustomRender = false; } // Not sure why you'd need this but I'll leave it here anyways

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

