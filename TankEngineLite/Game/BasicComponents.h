#ifndef BASIC_COMPONENTS
#define BASIC_COMPONENTS

#include <D3D.h>
#include "CoreComponents.h"
#include "InputManager.h"

class SpriteBatch;

//////////////////////////////////////////////////////////////////////////
// Lifespan component
class LifeSpan
	: public ECS::EntityComponent
{
public:
	LifeSpan() = default;
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
	bool m_MeetsRequirements;
	float m_Timer;
	float m_VerticalAcceleration;

	float m_SpriteTimer;
	int m_SpriteIndex;

	bool m_FacingRight;

	Player m_PlayerController;
};

#endif // !BASIC_COMPONENTS

