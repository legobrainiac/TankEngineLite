#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <D3D.h>
#include "CoreComponents.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "ColliderComponent.h"

class PlayerController
	: public ECS::EntityComponent
{
public:
	PlayerController() {}
	PlayerController(ECS::Entity* pE);

	void Update(float dt) override;
	void OnMessage(uint32_t message) override;
	inline void SetInputController(Player player) { m_PlayerController = player; }

	constexpr auto GetIsDead() const noexcept -> bool { return (int)m_StatE == 0; }
	constexpr auto GetHealth() const noexcept -> int { return m_Health; }
	constexpr auto GetScore() const noexcept -> int { return m_Score; }

	void Reset(bool score = false);

private:
	enum PlayerState
	{
		PLAYING,
		DEAD
	}m_StatE = PlayerState::PLAYING;

	void AliveUpdate(float dt);
	void DeadUpdate(float dt);
	void ParticleSphere();

	TransformComponent2D* m_pTransform = nullptr;
	SpriteRenderComponent* m_pRenderComponent = nullptr;
	ColliderComponent* m_pCollider = nullptr;
	XMFLOAT3 m_PreviousPosition{};

	FMOD::Sound* m_pShootingSound = nullptr;
	FMOD::Sound* m_pPickupSound = nullptr;
	FMOD::Sound* m_pDamage = nullptr;
	FMOD::Sound* m_pDeath = nullptr;

	// Timers
	float m_Timer = 0.f;
	float m_SpriteTimer = 0.f;
	float m_ParticleTimer = 0.f;
	
	int m_Health = 4;
	int m_Score = 0;
	float m_DamageTimer = 0.f;

	int m_SpriteIndex = 0;
	bool m_FacingRight = true;

	Player m_PlayerController = PLAYER1;
	bool m_MeetsRequirements = false;
};

#endif // !PLAYER_CONTROLLER