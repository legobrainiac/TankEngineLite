#ifndef BASIC_COMPONENTS
#define BASIC_COMPONENTS

#include <D3D.h>
#include "CoreComponents.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"

class SpriteBatch;

class Utils
{
public:
	template<typename T>
	static T Lerp(T start, T end, float percent)
	{
		return (start + percent * (end - start));
	}

	static int RandInterval(int min, int max)
	{
		return min + rand() % (max + 1 - min);
	}
};

//////////////////////////////////////////////////////////////////////////
// Lifespan component
class LifeSpan
	: public ECS::EntityComponent
{
public:
	LifeSpan()
		: m_LifeSpan(0.f)
		, m_Life(0.f)
	{
	}

	LifeSpan(ECS::Entity* pE);

	void Update(float dt) override;
	inline void SetLifeSpan(float span) { m_LifeSpan = span; }

private:
	float m_LifeSpan;
	float m_Life;
};

//////////////////////////////////////////////////////////////////////////
// Projectile Component
class ProjectileComponent
	: public ECS::EntityComponent
{
public:
	ProjectileComponent() = default;
	ProjectileComponent(ECS::Entity* pE);

	void Update(float dt) override;
	inline void SetDirection(const XMFLOAT2& direction) { m_Direction = direction; }
	inline void SetSpeed(float speed) { m_Speed = speed; }

private:
	TransformComponent2D* m_pTransform;
	XMFLOAT2 m_Direction;
	float m_Speed;
	bool m_MeetsRequirements;
};

//////////////////////////////////////////////////////////////////////////
// Player movement component
class PlayerController
	: public ECS::EntityComponent
{
public:
	PlayerController() {}
	PlayerController(ECS::Entity* pE);

	void Update(float dt) override;
	inline void SetInputController(Player player) { m_PlayerController = player; }

private:
	TransformComponent2D* m_pTransform;
	SpriteRenderComponent* m_pRenderComponent;

	FMOD::Sound* m_pShootingSound;

	// Timers
	float m_Timer;
	float m_SpriteTimer;

	float m_VerticalAcceleration;
	int m_SpriteIndex;
	bool m_FacingRight;

	Player m_PlayerController;
	bool m_MeetsRequirements;
};

//////////////////////////////////////////////////////////////////////////
// Particle emitter component
class ParticleEmitter
	: public ECS::EntityComponent
{
public:
	ParticleEmitter() = default;
	ParticleEmitter(ECS::Entity* pE);

	void Update(float dt) override;

	// Don't feel like making getters xD
	float m_ParticleSpawnInterval;
	float m_ParticleLifeTime;
	float m_Gravity;
	uint32_t m_ParticlesPerSpawn;
	SpriteBatch* m_pSpriteBatch;

	void SpawnParticles(float dt);

private:
	float m_Timer;
	bool m_MeetsRequirements;
	
	TransformComponent2D* m_pTransform;
};

//////////////////////////////////////////////////////////////////////////
// Particle emitter component
class Particle
	: public ECS::EntityComponent
{
public:
	Particle() = default;
	Particle(ECS::Entity* pE);

	void Update(float dt) override;
	void Initialize(SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startAcceleration, float scale, XMFLOAT4 colour, float life, float gravity);

private:
	float m_Timer;
	float m_Life;
	float m_Scale;
	float m_Gravity;
	XMFLOAT2 m_Pos;
	XMFLOAT2 m_Acceleration;
	XMFLOAT4 m_Colour;
	SpriteBatch* m_pSpriteBatch;
};

#endif // !BASIC_COMPONENTS

