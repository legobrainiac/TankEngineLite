#include "pch.h"
#include "BasicComponents.h"

#include "Prefabs.h"

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
PlayerController::PlayerController(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;

	m_pTransform = pE->GetComponent<TransformComponent2D>();
	m_pRenderComponent = pE->GetComponent<SpriteRenderComponent>();

	if (m_pTransform != nullptr && m_pRenderComponent != nullptr)
		m_MeetsRequirements = true;

	// Action mapping test
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_Q, ActionType::PRESSED,
			[this]()
			{
				m_pTransform->position = { 1600 / 2.f, 900.f / 2.f, 0.f };
			}));
}

void PlayerController::Update(float dt)
{
	if (!m_MeetsRequirements)
		return;

	// Simple movement code
	const float movementSpeed = 500.f;
	auto pInputMananager = InputManager::GetInstance();

	m_Timer += dt;
	m_SpriteTimer += dt;

	// Movement vector
	XMFLOAT2 movement{};

	// Input
	if (pInputMananager->IsPressed(ControllerButton::DPAD_LEFT, m_PlayerController))
		movement.x -= dt * movementSpeed;

	if (pInputMananager->IsPressed(ControllerButton::DPAD_RIGHT, m_PlayerController))
		movement.x += dt * movementSpeed;

	//////////////////////////////////////////////////////////////////////////
	// hardcoded physics

	// all of this is horrible code ik
	if(m_Jumped)
		m_RumbleTimer += dt;

	if (m_RumbleTimer > 0.25f)
	{
		pInputMananager->SetControlerRumble(0, 0, m_PlayerController);
		m_RumbleTimer -= m_RumbleTimer;
		m_Jumped = false;
	}

	m_VerticalAcceleration = (m_VerticalAcceleration + dt * 2.f * ( 0 - m_VerticalAcceleration));

	if (m_pTransform->position.y < 600.f - 32.f)
		movement.y += 1000.f * dt;
	else
	{
		if (pInputMananager->IsPressed(ControllerButton::A, m_PlayerController))
		{
			pInputMananager->SetControlerRumble(25000, 25000, m_PlayerController);
			m_VerticalAcceleration = -2000.f;
			m_Jumped = true;
		}
	}

	// hardcoded physics end
	//////////////////////////////////////////////////////////////////////////

	// Apply movement
	m_pTransform->position.x += movement.x;
	m_pTransform->position.y += movement.y + m_VerticalAcceleration * dt;

	// Direction
	m_FacingRight = (movement.x > 0.f);

	// Animations
	if (m_SpriteTimer > 0.05f)
	{
		// Sprite animation right
		constexpr XMFLOAT4 atlastTransformsRight[8]
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

		if (movement.x < -0.1f || movement.x > 0.1f)
		{
			auto atlasTransform = atlastTransformsRight[m_SpriteIndex];
			atlasTransform.y += m_PlayerController * 16;
			atlasTransform.w += m_PlayerController * 16;

			m_SpriteTimer -= m_SpriteTimer;
			m_SpriteIndex++;

			if (m_SpriteIndex == 8)
				m_SpriteIndex = 0;

			// Which of the sprite should you use
			if(!m_FacingRight)
			{
				atlasTransform.x += 128;
				atlasTransform.z += 128;
			}

			m_pRenderComponent->SetAtlasTransform(atlasTransform);
		}
		else
		{
			auto atlasTransform = atlastTransformsRight[0];
			atlasTransform.y += m_PlayerController * 16;
			atlasTransform.w += m_PlayerController * 16;

			m_pRenderComponent->SetAtlasTransform(atlasTransform);
			m_FacingRight = true; // Hacky
		}
	}

	if (pInputMananager->IsPressed(ControllerButton::B, m_PlayerController))
	{
		if (m_Timer > 0.2f)
		{
			const auto pWorld = m_pOwner->GetWorld();
			const auto pos = m_pTransform->position;

			const XMFLOAT2 direction = (m_FacingRight) ? XMFLOAT2{ 1.f, 0.f } : XMFLOAT2{ -1.f, 0.f };

			Prefabs::CreateBubbleProjectile(
				pWorld,
				{ pos.x, pos.y },
				direction,
				m_pRenderComponent->GetSpriteBatch()
			);

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
