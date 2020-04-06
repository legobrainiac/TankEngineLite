#include "pch.h"
#include "BasicComponents.h"
#include "Texture2D.h"

// Render Component
void RenderComponent::Render() const
{
	// Only render if all dependencies are available
	if (!m_MeetsRequirements)
		return;
    
    // If using a custom render callback, use that one instead of the normal one
    if(m_bShouldCustomRender)
    {
        m_CustomRenderFunction(m_pOwner);
        return;
    }
    
	// TODO(tomas): change transform to dxmath
	// TODO(tomas): dont hard core scale, also add that to transform
	m_pSpriteBatch->PushSprite(
		m_AtlasTransform, 
		XMFLOAT3{ m_pTransform->position.x, m_pTransform->position.y, m_pTransform->position.z }, 
		m_pTransform->rotation, 
		{ 4.f, 4.f }, 
		{ 0.5f, 0.5f }, 
		{ 1.f, 1.f, 1.f, 1.f }
	);
}

inline void RenderComponent::SetCustomRenderFunction(const CustomRenderFunction& crf)
{
    m_CustomRenderFunction = crf;
    m_bShouldCustomRender = true;
}

RenderComponent::~RenderComponent()
{
}