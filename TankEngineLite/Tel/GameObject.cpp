#include "TelPCH.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "Renderer.h"

GameObject::~GameObject()
{
    delete m_pTexture;
}

void GameObject::Update(){}

void GameObject::Render() const
{
	const auto pos = m_Transform.GetPosition();
	Renderer::GetInstance()->RenderTexture(m_pTexture, pos.x, pos.y);
}

void GameObject::SetTexture(const std::string& filename)
{
	m_pTexture = ResourceManager::GetInstance()->LoadTexture(filename);
}

void GameObject::SetPosition(float x, float y)
{
	m_Transform.SetPosition(x, y, 0.0f);
}
