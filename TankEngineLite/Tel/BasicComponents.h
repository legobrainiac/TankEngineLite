#ifndef BASIC_COMPONENTS
#define BASIC_COMPONENTS

#pragma warning(push)
#pragma warning (disable:4201)
#include <glm/vec3.hpp>
#pragma warning(pop)

#include <functional>

#include <SDL.h>
#include <SDL_ttf.h>

#include "ecs.h"
#include "Font.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "InputManager.h"

class Texture2D;

// Transform component
class TransformComponent
    : public ECS::EntityComponent
{
public:
	TransformComponent() {}
    ~TransformComponent() { }

	virtual void CleanInitialize(ECS::Entity* pE) override
	{
		m_pOwner = pE;
		position = { 0.f, 0.f, 0.f };
		rotation = 0.f;
	}
    
    // These are made public for ease of access and manipulation
    glm::vec3 position;
    float rotation;
};

/*
// Lifespan component
class LifeSpan
    : public ECS::EntityComponent
{
public:
	LifeSpan() {}
	LifeSpan(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
		, m_LifeSpan{ -1.f }
        , m_Life{}
	{
	}
    
	void Update(float dt) override
	{
		m_Life += dt;
        
		if (m_Life > m_LifeSpan)
			m_pOwner->GetWorld()->DestroyEntity(m_pOwner->GetID());
	}
    
	inline void SetLifeSpan(float span) { m_LifeSpan = span; }
    
private:
	float m_LifeSpan;
	float m_Life;
};*/

// Render Component
typedef std::function<void(ECS::Entity*)> CustomRenderFunction;

class RenderComponent
    : public ECS::EntityComponent
{
public:
	virtual void CleanInitialize(ECS::Entity* pE) override
	{
		m_pOwner = pE;
		pTexture = nullptr;
		m_MeetsRequirements = false;
		m_bShouldCustomRender = false;
		m_pTransform = pE->GetComponent<TransformComponent>();

		if (m_pTransform != nullptr)
			m_MeetsRequirements = true;
	}

	RenderComponent() {}

	// TODO(tomas): THIS IS VERY TEMPORARY, NO GAMEOBJECT, ENTITY, COMPONENENT ANYTHING SHOULD OWN ANY SORT OF RESOURCE
	~RenderComponent();
    
    inline void SetTexture(Texture2D* pTex) { pTexture = pTex; }
    void Render() const;
    
	// These are made public for ease of access and manipulation
    Texture2D* pTexture;
    
    // Custom rendering function
    // Can also be used if you need to inject something in to the rendering loop
    inline void SetCustomRenderFunction(const CustomRenderFunction& crf);
    inline void ResetToDefault() { m_bShouldCustomRender = false; } // Not sure why you'd need this but i'll leave it here anyways
    
private:
    TransformComponent* m_pTransform;
    bool m_MeetsRequirements;    
    
    // Custom rendering
    CustomRenderFunction m_CustomRenderFunction;
    bool m_bShouldCustomRender;
};

// Text Component
class TextComponent
	: public ECS::EntityComponent
{
public:
	virtual void CleanInitialize(ECS::Entity* pE) override
	{
		m_pOwner = pE;
		m_pRenderComponent = nullptr;
		m_MeetsRequirements = false;
		m_NeedsUpdate = false;

		m_pRenderComponent = pE->GetComponent<RenderComponent>();

		if (m_pRenderComponent != nullptr)
			m_MeetsRequirements = true;
	}

	TextComponent() {}

	void Initialize(const std::string& text, Font* pFont, const SDL_Color& color = { 255, 255, 255 })
	{
		m_Text = text;
		m_pFont = pFont;
		m_Color = color;
	}

	~TextComponent()
	{
		if (m_pFont != nullptr && !m_IsDirty)
			delete m_pFont;
	}

	void Update(float dt) override
	{
		(void)dt;

		if (!m_NeedsUpdate)
			return;

		const auto surf = TTF_RenderText_Blended(m_pFont->GetFont(), m_Text.c_str(), m_Color);
		if (surf == nullptr)
			throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());

		auto texture = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), surf);
		if (texture == nullptr)
			throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());

		SDL_FreeSurface(surf);
		delete m_pRenderComponent->pTexture;
		m_pRenderComponent->pTexture = new Texture2D(texture);
		m_NeedsUpdate = false;
	}

	inline void SetText(const std::string& text, const SDL_Color& color = { 255,255,255 })
	{
		m_Text = text;
		m_NeedsUpdate = true;
		m_Color = color;
	}

	inline void SetFont(Font* pFont) { m_pFont = pFont; }

private:
	RenderComponent* m_pRenderComponent;
	Font* m_pFont;

	SDL_Color m_Color;
	std::string m_Text;
	bool m_MeetsRequirements;
	bool m_NeedsUpdate;
};

class MovementComponent
    : public ECS::EntityComponent
{
public:
	virtual void CleanInitialize(ECS::Entity* pE) override
	{
		m_pOwner = pE;
		m_MeetsRequirements = false;

		m_pTransform = pE->GetComponent<TransformComponent>();

		if (m_pTransform != nullptr)
			m_MeetsRequirements = true;
	}

	MovementComponent() {}
    
    void Update(float dt)
    {
        if(!m_MeetsRequirements) 
            return;
        
        // Simple movement code
        const float movementSpeed = 500.f;
        auto pInputMananager = InputManager::GetInstance();
        
        if(pInputMananager->IsKeyDown(SDL_SCANCODE_W))
            m_pTransform->position.y -= dt * movementSpeed;
        
        if(pInputMananager->IsKeyDown(SDL_SCANCODE_S))
            m_pTransform->position.y += dt * movementSpeed;
        
        if(pInputMananager->IsKeyDown(SDL_SCANCODE_A))
            m_pTransform->position.x -= dt * movementSpeed;
        
        if(pInputMananager->IsKeyDown(SDL_SCANCODE_D))
            m_pTransform->position.x += dt * movementSpeed;
    }
    
private:
    TransformComponent* m_pTransform;
    bool m_MeetsRequirements;        
};

#endif // !BASIC_COMPONENTS

