#ifndef MAITA_CONTROLLER_H
#define MAITA_CONTROLLER_H

#include "CoreComponents.h"
#include "ColliderComponent.h"

#include <array>

class MaitaController 
	: public ECS::EntityComponent
{
public:
	MaitaController() = default;
	MaitaController(ECS::Entity* pE);

	void Update(float dt);
	void OnMessage(uint32_t message);
	void SetPlayers(const std::array<TransformComponent2D*, 4>& playerTransforms) noexcept { m_Players = playerTransforms; }

private:
	enum ZCStates
	{
		NORMAL,
		IN_BUBBLE
	}m_State = ZCStates::NORMAL;
	float m_BubbleTimer = 0.f;

	TransformComponent2D* m_pTransform;
	SpriteRenderComponent* m_pRenderComponent;
	ColliderComponent* m_pCollider;

	std::array<TransformComponent2D*, 4> m_Players;
	
	const float m_MovingSpeed = 100.f;
	float m_BolderTimer = 1.f;

	float m_SpriteTimer = 0.f;
	int m_SpriteIndex = 0;
	bool m_FacingRight = true;
	
	bool m_MeetsRequirements;
};

#endif // !MAITA_CONTROLLER_H