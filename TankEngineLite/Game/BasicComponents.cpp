#include "pch.h"
#include "BasicComponents.h"

#include "Prefabs.h"

#include "BBLevel.h"
#include "SpriteBatch.h"

#include "Sound.h"

#include "PlayerController.h"

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
// Projectile component
ProjectileComponent::ProjectileComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;
	m_pTransform = pE->GetComponent<TransformComponent2D>();
	m_pCollider = pE->GetComponent<ColliderComponent>();

	if (m_pTransform != nullptr && m_pCollider != nullptr)
		m_MeetsRequirements = true;
}

void ProjectileComponent::Update([[maybe_unused]] float dt)
{
	m_pCollider->AddMovement(
		{
			m_Direction.x * m_Speed,
			m_Direction.y * m_Speed,
		});
}

//////////////////////////////////////////////////////////////////////////
// Particle Emitter component
ParticleEmitter::ParticleEmitter(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
	, m_ShouldSpawn(true)
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
	if (!m_ShouldSpawn)
		return;

	const XMFLOAT2 pos = { m_pTransform->position.x, m_pTransform->position.y };
	for (uint32_t i = 0; i < m_ParticlesPerSpawn; ++i)
	{
		float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;

		XMFLOAT2 accel{ cosf(angle) * 500.f, sinf(angle) * 500.f };

		Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pSpriteBatch, pos, accel, { 1.f, .3f, 0.3f, 1.f }, m_ParticleLifeTime, m_Gravity);
	}
}

//////////////////////////////////////////////////////////////////////////
// Particle
Particle::Particle(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
	, m_Timer()
	, m_Life()
	, m_Scale()
	, m_Gravity()
	, m_Pos()
	, m_Acceleration()
	, m_Colour()
	, m_pSpriteBatch()
{
	m_Timer = 0;
}

void Particle::Update(float dt)
{
	m_Timer += dt;

	if (m_Timer > m_Life)
		m_pOwner->GetWorld()->AsyncDestroyEntity(m_pOwner->GetId());

	// SIMD, should actually test if theres a performance impact 
	m_Pos.x += m_Acceleration.x * dt;
	m_Pos.y += m_Acceleration.y * dt;

	m_Acceleration.x = Utils::Lerp(m_Acceleration.x, 0.f, dt);
	m_Acceleration.y = Utils::Lerp(m_Acceleration.y, m_Gravity, dt);

	XMFLOAT4 colour = m_Colour;
	m_Colour.y = abs(cosf(m_Timer * 3.f));
	m_Colour.x = abs(sinf(m_Timer * 3.f));

	float scale = m_Scale - ((m_Scale / m_Life) * m_Timer);
	m_pSpriteBatch->PushSprite({ 8, 128 - 16, 12, 132 - 16 }, { m_Pos.x, m_Pos.y, 0 }, 0, { scale, scale }, { 0.5f, 0.5f }, m_Colour);
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
