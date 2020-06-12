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

private:
	TransformComponent2D* m_pTransform;
	SpriteRenderComponent* m_pRenderComponent;
	ColliderComponent* m_pCollider;
	XMFLOAT3 m_PreviousPosition;

	FMOD::Sound* m_pShootingSound;

	// Timers
	float m_Timer;
	float m_SpriteTimer;
	
	int m_Health = 4;
	int m_Score = 0;
	float m_DamageTimer;

	int m_SpriteIndex;
	bool m_FacingRight;

	Player m_PlayerController;
	bool m_MeetsRequirements;
};

#endif // !PLAYER_CONTROLLER