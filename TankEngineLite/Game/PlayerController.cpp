#include "PlayerController.h"
#include "Prefabs.h"
#include "Sound.h"

//////////////////////////////////////////////////////////////////////////
// Movement Component
PlayerController::PlayerController(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;

	m_pTransform = pE->GetComponent<TransformComponent2D>();
	m_pRenderComponent = pE->GetComponent<SpriteRenderComponent>();
	m_pCollider = pE->GetComponent<ColliderComponent>();

	if (m_pTransform != nullptr && m_pRenderComponent != nullptr && m_pCollider != nullptr)
		m_MeetsRequirements = true;

	// Load sound
	m_pShootingSound = RESOURCES->Get<Sound>("blep")->GetSound();
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
	m_PreviousPosition = m_pTransform->position;

	// Movement vector
	XMFLOAT2 movement{};

	// Input
	if (pInputMananager->IsPressed(ControllerButton::DPAD_LEFT, m_PlayerController))
		movement.x -= movementSpeed;

	if (pInputMananager->IsPressed(ControllerButton::DPAD_RIGHT, m_PlayerController))
		movement.x += movementSpeed;

	if (pInputMananager->IsPressed(ControllerButton::A, m_PlayerController) && m_pCollider->IsGrounded())
	{
		// Spawn particles for jumping
		const XMFLOAT2 pos = { m_pTransform->position.x + 32.f, m_pTransform->position.y + 64.f };
		for (uint32_t i = 0; i < 10; ++i)
		{
			float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

			XMFLOAT2 accel{ cosf(angle) * 100.f, -abs(sin(angle)) * 50.f };
			Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), pos, accel, { 0.0f, 0.6f, 1.0f, 1.f }, 0.5f, 50.f);
		}

		// Actually jump
		m_pCollider->AddAcceleration({ 0.f, -1750.f });
		m_pCollider->SetIsGrounded(false);
	}
	
	m_FacingRight = (movement.x >= 0.f);
	m_pCollider->AddMovement(movement);

	// Shooting
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
				m_pRenderComponent->GetSpriteBatch(),
				m_pCollider->GetLevel()
			);

			m_Timer -= m_Timer;

			// Rumble in the firing direction
			if (m_FacingRight)
				InputManager::GetInstance()->RumbleController(0, 35000, 0.1f, m_PlayerController);
			else
				InputManager::GetInstance()->RumbleController(35000, 0, 0.1f, m_PlayerController);

			const auto sound = SOUND->Play(m_pShootingSound);
			sound->setVolume(0.5f);
		}
	}

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
			if (!m_FacingRight)
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
			m_FacingRight = true;
		}
	}
}