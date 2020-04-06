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
#include "Texture.h"
#include "InputManager.h"
#include "ResourceManager.h"

#include "SpriteBatch.h"

class Texture;

// Transform component
class TransformComponent
	: public ECS::EntityComponent
{
public:
	TransformComponent() {}
	TransformComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		position = { 0.f, 0.f, 0.f };
		rotation = 0.f;
	}

	~TransformComponent() { }

	// These are made public for ease of access and manipulation
	glm::vec3 position;
	float rotation;
};

// Lifespan component
class LifeSpan
	: public ECS::EntityComponent
{
public:
	LifeSpan() {}
	LifeSpan(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_LifeSpan = -1.f;
		m_Life = 0.f;
	}

	void Update(float dt) override
	{
		m_Life += dt;

		if (m_Life > m_LifeSpan)
			m_pOwner->GetWorld()->DestroyEntity(m_pOwner->GetId());
	}

	inline void SetLifeSpan(float span) { m_LifeSpan = span; }

private:
	float m_LifeSpan;
	float m_Life;
};

// Render Component
typedef std::function<void(ECS::Entity*)> CustomRenderFunction;

class RenderComponent
	: public ECS::EntityComponent
{
public:
	RenderComponent() {}
	RenderComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_pSpriteBatch = nullptr;
		m_MeetsRequirements = false;
		m_bShouldCustomRender = false;
		m_pTransform = pE->GetComponent<TransformComponent>();

		if (m_pTransform != nullptr)
			m_MeetsRequirements = true;
	}

	~RenderComponent();

	inline void SetSpriteBatch(SpriteBatch* pBatch) { m_pSpriteBatch = pBatch; }
	inline SpriteBatch* GetSpriteBatch() const { return m_pSpriteBatch; }

	inline void SetAtlasTransform(const XMFLOAT4& transform) { m_AtlasTransform = transform; }
	void Render() const;

	// Custom rendering function
	// Can also be used if you need to inject something in to the rendering loop
	inline void SetCustomRenderFunction(const CustomRenderFunction& crf);
	inline void ResetToDefault() { m_bShouldCustomRender = false; } // Not sure why you'd need this but i'll leave it here anyways

private:
	TransformComponent* m_pTransform;
	SpriteBatch* m_pSpriteBatch;
	XMFLOAT4 m_AtlasTransform;
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
	TextComponent() {}
	TextComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_pRenderComponent = nullptr;
		m_MeetsRequirements = false;
		m_NeedsUpdate = false;

		m_pRenderComponent = pE->GetComponent<RenderComponent>();

		if (m_pRenderComponent != nullptr)
			m_MeetsRequirements = true;
	}

	void Initialize(const std::string& text, Font* pFont, const SDL_Color& color = { 255, 255, 255 })
	{
		m_Text = text;
		m_pFont = pFont;
		m_Color = color;
	}

	~TextComponent()
	{
		if (m_pFont != nullptr)
			delete m_pFont;
	}

	void Update(float dt) override
	{
		(void)dt;

#if 0
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
		m_pRenderComponent->pTexture = new Texture(texture);
		m_NeedsUpdate = false;
#endif
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
	MovementComponent() {}
	MovementComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_MeetsRequirements = false;

		m_pTransform = pE->GetComponent<TransformComponent>();
		m_pRenderComponent = pE->GetComponent<RenderComponent>();

		if (m_pTransform != nullptr && m_pRenderComponent != nullptr)
			m_MeetsRequirements = true;
	}

	void Update(float dt) override
	{
		if (!m_MeetsRequirements)
			return;

		// Simple movement code
		const float movementSpeed = 500.f;
		auto pInputMananager = InputManager::GetInstance();

		m_pTransform->rotation += dt;
		m_Timer += dt;
		m_SpriteTimer += dt;

		// Sprite animation
		constexpr XMFLOAT4 atlastTransforms[8]
		{
			{ 0, 0, 16, 16 },
			{ 16, 0, 32, 16 },
			{ 32, 0, 48, 16 },
			{ 48, 0, 64, 16 },
			{ 64, 0, 80, 16 },
			{ 80, 0, 96, 16 },
			{ 96, 0, 112, 16 },
			{ 112, 0, 128, 16 }
		};

		if (m_SpriteTimer > 0.05f)
		{
			m_SpriteTimer -= m_SpriteTimer;
			m_SpriteIndex++;

			if (m_SpriteIndex == 8)
				m_SpriteIndex = 0;

			m_pRenderComponent->SetAtlasTransform(atlastTransforms[m_SpriteIndex]);
		}
		
		// Input
		if (pInputMananager->IsKeyDown(SDL_SCANCODE_W) || pInputMananager->IsPressed(ControllerButton::DPAD_UP))
			m_pTransform->position.y -= dt * movementSpeed;

		if (pInputMananager->IsKeyDown(SDL_SCANCODE_S) || pInputMananager->IsPressed(ControllerButton::DPAD_DOWN))
			m_pTransform->position.y += dt * movementSpeed;

		if (pInputMananager->IsKeyDown(SDL_SCANCODE_A) || pInputMananager->IsPressed(ControllerButton::DPAD_LEFT))
			m_pTransform->position.x -= dt * movementSpeed;

		if (pInputMananager->IsKeyDown(SDL_SCANCODE_D) || pInputMananager->IsPressed(ControllerButton::DPAD_RIGHT))
			m_pTransform->position.x += dt * movementSpeed;

		if (pInputMananager->IsPressed(ControllerButton::START))
			m_pOwner->GetWorld()->DestroyEntity(m_pOwner->GetId());

		if (pInputMananager->IsPressed(ControllerButton::RIGHT_THUMB))
		{
			if (m_Timer > 0.01f)
			{
				auto pEntity = m_pOwner->GetWorld()->CreateEntity();
				auto [pLifeSpan, pRenderer, pTransform] = pEntity->PushComponents<LifeSpan, RenderComponent, TransformComponent>();

				pTransform->position = m_pTransform->position;
				pTransform->rotation = m_pTransform->rotation;
				pRenderer->SetSpriteBatch(m_pRenderComponent->GetSpriteBatch());
				pRenderer->SetAtlasTransform(atlastTransforms[m_SpriteIndex]);
				pLifeSpan->SetLifeSpan(5.f);

				m_Timer -= m_Timer;
			}
		}
	}

private:
	TransformComponent* m_pTransform;
	RenderComponent* m_pRenderComponent;
	bool m_MeetsRequirements;
	float m_Timer;

	float m_SpriteTimer;
	int m_SpriteIndex;
};

#endif // !BASIC_COMPONENTS

