#include "pch.h"
#include "BasicComponents.h"

#include "Prefabs.h"

#include "SpriteBatch.h"

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

	// Load sound
	m_pShootingSound = ResourceManager::GetInstance()->GetSound("blep");
	if (!m_pShootingSound)
		m_pShootingSound = ResourceManager::GetInstance()->LoadSound("blep.wav", "blep");
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
	// hard coded physics

	m_VerticalAcceleration = (m_VerticalAcceleration + dt * 2.f * (0 - m_VerticalAcceleration));

	if (m_pTransform->position.y < 600.f - 32.f)
	{
		float totalMovement = 981.f * dt;
		movement.y += totalMovement;
	}
	else
	{
		if (pInputMananager->IsPressed(ControllerButton::A, m_PlayerController))
		{
			// Spawn particles for jumping
			const XMFLOAT2 pos = { m_pTransform->position.x, m_pTransform->position.y + 32.f };
			for (uint32_t i = 0; i < 10; ++i)
			{
				float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

				XMFLOAT2 accel{ cosf(angle) * 100.f, -abs(sin(angle)) * 50.f };
				Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), pos, accel, { 0.0f, 0.6f, 1.0f, 1.f }, 0.5f, 50.f);
			}

			// Actually jump
			m_VerticalAcceleration = -2000.f;
		}
	}

	// hard coded physics end
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

			// Rumble in the firing direction
			if (m_FacingRight)
				InputManager::GetInstance()->RumbleController(0, 35000, 0.1f, m_PlayerController);
			else
				InputManager::GetInstance()->RumbleController(35000, 0, 0.1f, m_PlayerController);

			const auto sound = SoundManager::GetInstance()->PlaySound(m_pShootingSound);
			sound->setVolume(0.5f);
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

//////////////////////////////////////////////////////////////////////////
// Particle Emitter component
ParticleEmitter::ParticleEmitter(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;
	m_pTransform = pE->GetComponent<TransformComponent2D>();

	if (m_pTransform != nullptr)
		m_MeetsRequirements = true;
}

void ParticleEmitter::Update(float dt)
{
	if (!m_MeetsRequirements)
		return;

	m_Timer += dt;

	if (m_Timer > m_ParticleSpawnInterval)
	{
		SpawnParticles(dt);
		m_Timer -= m_Timer;
	}
}

void ParticleEmitter::SpawnParticles([[maybe_unused]] float dt)
{
	const XMFLOAT2 pos = { m_pTransform->position.x, m_pTransform->position.y };
	for (uint32_t i = 0; i < m_ParticlesPerSpawn; ++i)
	{
		float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

		XMFLOAT2 accel{ cosf(angle) * 50.f, sinf(angle) * 50.f };

		Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pSpriteBatch, pos, accel, { 1.f, .3f, 0.3f, 1.f }, m_ParticleLifeTime, m_Gravity);
	}
}

//////////////////////////////////////////////////////////////////////////
// Particle
Particle::Particle(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_Timer = 0;
}

void Particle::Update(float dt)
{
	m_Timer += dt;

	if (m_Timer > m_Life)
		m_pOwner->GetWorld()->DestroyEntity(m_pOwner->GetId());

	// SIMD, should actually test if theres a performance impact 
	m_Pos.x += m_Acceleration.x * dt;
	m_Pos.y += m_Acceleration.y * dt;

	m_Acceleration.x = Utils::Lerp(m_Acceleration.x, 0.f, dt);
	m_Acceleration.y = Utils::Lerp(m_Acceleration.y, m_Gravity, dt);

	XMFLOAT4 colour = m_Colour;
	m_Colour.y = abs(cosf(m_Timer * 3.f));
	m_Colour.x = abs(sinf(m_Timer * 3.f));

	m_pSpriteBatch->PushSprite({ 8, 128, 12, 132 }, { m_Pos.x, m_Pos.y, 0 }, 0, { m_Scale , m_Scale }, { 0.5f, 0.5f }, m_Colour);
}

void Particle::Initialize(SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startAcceleration, float scale, XMFLOAT4 colour, float life, float gravity)
{
	m_pSpriteBatch = pSpriteBatch;
	m_Pos = pos;
	m_Acceleration = startAcceleration;
	m_Scale = scale;
	m_Colour = colour;
	m_Life = life;
	m_Gravity = gravity;
}
