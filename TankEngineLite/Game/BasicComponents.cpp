#include "BasicComponents.h"

#include "CoreComponents.h"
#include "InputManager.h"

//////////////////////////////////////////////////////////////////////////
// Life span component
LifeSpan::LifeSpan(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_LifeSpan = -1.f;
	m_Life = 0.f;
}

void LifeSpan::Update(float dt)
{
	m_Life += dt;

	if (m_Life > m_LifeSpan)
		m_pOwner->GetWorld()->DestroyEntity(m_pOwner->GetId());
}

//////////////////////////////////////////////////////////////////////////
// Movement Component
MovementComponent::MovementComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;

	m_pTransform = pE->GetComponent<TransformComponent2D>();
	m_pRenderComponent = pE->GetComponent<SpriteRenderComponent>();

	if (m_pTransform != nullptr && m_pRenderComponent != nullptr)
		m_MeetsRequirements = true;
}

void MovementComponent::Update(float dt)
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

	if (pInputMananager->IsPressed(ControllerButton::A))
	{
		if (m_Timer > 0.1f)
		{
			auto pEntity = m_pOwner->GetWorld()->CreateEntity();
			auto [pLifeSpan, pRenderer, pProjectile, pTransform] = pEntity->PushComponents<LifeSpan, SpriteRenderComponent, ProjectileComponent, TransformComponent2D>();

			pTransform->position = m_pTransform->position;
			pTransform->position.z = 0.1f;
			pTransform->scale = { 4.f, 4.f };
			pRenderer->SetSpriteBatch(m_pRenderComponent->GetSpriteBatch());
			pRenderer->SetAtlasTransform({ 0, 224,  16, 240 });
			pLifeSpan->SetLifeSpan(2.f);

			pProjectile->SetDirection({ cosf(m_pTransform->rotation), sinf(m_pTransform->rotation) });
			pProjectile->SetSpeed(1000.f);

			m_Timer -= m_Timer;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Projectile component
ProjectileComponent::ProjectileComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;
	m_pTransform = pE->GetComponent<TransformComponent2D>();

	if (m_pTransform != nullptr)
		m_MeetsRequirements = true;
}

void ProjectileComponent::Update(float dt)
{
	m_pTransform->position.x += m_Direction.x * dt * m_Speed;
	m_pTransform->position.y += m_Direction.y * dt * m_Speed;
}
