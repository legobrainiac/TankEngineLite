#include "PlayerController.h"
#include "MainGame.h"
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
	if (!m_MeetsRequirements || !MainGame::IsRunning)
		return;

	switch (m_StatE)
	{
	case PlayerController::PLAYING:
		AliveUpdate(dt);
		break;
	case PlayerController::DEAD:
		DeadUpdate(dt);
		break;
	}
}

void PlayerController::OnMessage(uint32_t message)
{
	// Take Damage
	if (m_DamageTimer > 1.f && message == 0U)
	{
		--m_Health;
		m_DamageTimer = 0.f;
		LOGGER->Log<LOG_INFO>("TOOK DAMAGE");

		if (m_Health <= 0U)
			m_StatE = PlayerState::DEAD;

		auto pos = m_pTransform->position;
		pos.x += 32.f;
		pos.y += 32.f;

		InputManager::GetInstance()->RumbleController(35000, 35000, 0.2f, m_PlayerController);
		ParticleSphere();
	}

	// Burst bubble or eat water melon gives you 100 points
	if (message == 1U)
	{
		LOGGER->Log<LOG_INFO>("100 SCORE");
		InputManager::GetInstance()->RumbleController(35000, 35000, 0.2f, m_PlayerController);
		m_Score += 100;
		ParticleSphere();
		// TODO(tomas): spawn hit marker for 100 score
	}

	// Pizza
	if (message == 2U)
	{
		LOGGER->Log<LOG_INFO>("200 SCORE");
		InputManager::GetInstance()->RumbleController(35000, 35000, 0.2f, m_PlayerController);
		m_Score += 200;
		ParticleSphere();
		// TODO(tomas): spawn hit marker for 100 score
	}
}

void PlayerController::AliveUpdate(float dt)
{
	constexpr SDL_Scancode lookupTableJump[2]{ SDL_SCANCODE_W, SDL_SCANCODE_UP };
	constexpr SDL_Scancode lookupTableRight[2]{ SDL_SCANCODE_D, SDL_SCANCODE_RIGHT };
	constexpr SDL_Scancode lookupTableLeft[2]{ SDL_SCANCODE_A, SDL_SCANCODE_LEFT };
	constexpr SDL_Scancode lookupTableUse[2]{ SDL_SCANCODE_SPACE, SDL_SCANCODE_RCTRL };

	m_DamageTimer += dt;
	m_pRenderComponent->SetEnabled(true);

	// Simple movement code
	const float movementSpeed = 500.f;
	auto pInputMananager = InputManager::GetInstance();

	m_Timer += dt;
	m_SpriteTimer += dt;
	m_PreviousPosition = m_pTransform->position;

	// Movement vector
	XMFLOAT2 movement{};

	// Input
	if (pInputMananager->IsPressed(ControllerButton::DPAD_LEFT, m_PlayerController) || 
		pInputMananager->IsKeyDown(lookupTableLeft[m_PlayerController]))
		movement.x -= movementSpeed;

	if (pInputMananager->IsPressed(ControllerButton::DPAD_RIGHT, m_PlayerController) ||
		pInputMananager->IsKeyDown(lookupTableRight[m_PlayerController]))
		movement.x += movementSpeed; 

	if ((pInputMananager->IsPressed(ControllerButton::A, m_PlayerController) || 
		pInputMananager->IsKeyDown(lookupTableJump[m_PlayerController])) && 
		m_pCollider->IsGrounded())
	{
		
		// Spawn particles for jumping
		const XMFLOAT2 pos = { m_pTransform->position.x + 32.f, m_pTransform->position.y + 64.f };
		for (uint32_t i = 0; i < 10; ++i)
		{
			float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

			XMFLOAT2 accel{ cosf(angle) * 100.f, -abs(sin(angle)) * 50.f };
			Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), pos, accel, { 0.0f, 1.f, .0f, 1.f }, 0.5f, 50.f);
		}

		// Actually jump
		m_pCollider->AddAcceleration({ 0.f, -1750.f });
		m_pCollider->SetIsGrounded(false);
	}

	m_FacingRight = (movement.x >= 0.f);
	m_pCollider->AddMovement(movement);

	// Shooting
	if (pInputMananager->IsPressed(ControllerButton::B, m_PlayerController) ||
		pInputMananager->IsKeyDown(lookupTableUse[m_PlayerController])
		)
	{
		if (m_Timer > 0.2f)
		{
			const auto pWorld = m_pOwner->GetWorld();
			const auto pos = m_pTransform->position;

			const XMFLOAT2 direction = (m_FacingRight) ? XMFLOAT2{ 1.f, 0.f } : XMFLOAT2{ -1.f, 0.f };

			Prefabs::CreateBubbleProjectile(
				pWorld,
				{ pos.x + 32.f, pos.y + 32.f },
				direction,
				m_pRenderComponent->GetSpriteBatch()
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

void PlayerController::DeadUpdate([[maybe_unused]] float dt)
{
	m_pRenderComponent->SetEnabled(false);
}

void PlayerController::ParticleSphere()
{
	const XMFLOAT2 pos = { m_pTransform->position.x + 32.f, m_pTransform->position.y + 64.f };
	for (uint32_t i = 0; i < 10; ++i)
	{
		float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

		XMFLOAT2 accel{ cosf(angle) * 100.f, -abs(sin(angle)) * 50.f };
		Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), pos, accel, { 0.0f, 1.f, .0f, 1.f }, 0.5f, 50.f);
	}
}
