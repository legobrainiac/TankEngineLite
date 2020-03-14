#include "TelPCH.h"
#include "BasicComponents.h"
#include "Texture2D.h"

// Render Component
void RenderComponent::Render() const
{
	// Only render if all dependencies are available
	if (!m_MeetsRequirements || !pTexture)
		return;

	Renderer::GetInstance()->RenderTexture(pTexture, m_pTransform->position.x, m_pTransform->position.y);
}

RenderComponent::~RenderComponent()
{
	delete pTexture;
}