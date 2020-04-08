#include "CoreComponents.h"

#include "SpriteBatch.h"
#include "Renderer.h"

// Render Component
void SpriteRenderComponent::Render() const
{
	// Only render if all dependencies are available
	if (!m_MeetsRequirements)
		return;

	// If using a custom render callback, use that one instead of the normal one
	if (m_bShouldCustomRender)
	{
		m_CustomRenderFunction(m_pOwner);
		return;
	}

	m_pSpriteBatch->PushSprite(
		m_AtlasTransform,
		m_pTransform->position,
		m_pTransform->rotation,
		m_pTransform->scale,
		{ 0.5f, 0.5f },
		{ 1.f, 1.f, 1.f, 1.f }
	);
}

inline void SpriteRenderComponent::SetCustomRenderFunction(const CustomRenderFunction& crf)
{
	m_CustomRenderFunction = crf;
	m_bShouldCustomRender = true;
}

SpriteRenderComponent::SpriteRenderComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_pSpriteBatch = nullptr;
	m_MeetsRequirements = false;
	m_bShouldCustomRender = false;
	m_pTransform = pE->GetComponent<TransformComponent2D>();

	if (m_pTransform != nullptr)
		m_MeetsRequirements = true;
}