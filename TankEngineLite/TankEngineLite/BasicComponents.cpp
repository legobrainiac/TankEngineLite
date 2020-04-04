#include "pch.h"
#include "BasicComponents.h"
#include "Texture2D.h"

// Render Component
void RenderComponent::Render() const
{
	// Only render if all dependencies are available
	if (!m_MeetsRequirements || !pTexture)
		return;
    
    // If using a custom render callback, use that one instead of the normal one
    if(m_bShouldCustomRender)
    {
        m_CustomRenderFunction(m_pOwner);
        return;
    }
    
    // If we're here, default render behaviour is being called
	Renderer::GetInstance()->RenderTexture(pTexture, m_pTransform->position.x, m_pTransform->position.y);
}

inline void RenderComponent::SetCustomRenderFunction(const CustomRenderFunction& crf)
{
    m_CustomRenderFunction = crf;
    m_bShouldCustomRender = true;
}

RenderComponent::~RenderComponent()
{
	if (pTexture != nullptr)
		delete pTexture;
}