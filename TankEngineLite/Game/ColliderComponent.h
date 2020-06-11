#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H

#include <D3D.h>
#include "CoreComponents.h"

class BBLevel;

class ColliderComponent
	: public ECS::EntityComponent
{
public:
	ColliderComponent() = default;

	ColliderComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_pTransform = pE->GetComponent<TransformComponent2D>();
		m_pColliderSystem = m_pOwner->GetWorld()->GetSystemByComponent<ColliderComponent>();

		if (m_pTransform && m_pColliderSystem)
			m_MeetsRequirements = true;
	}

	inline void AddMovement(const XMFLOAT2& movement) noexcept 
	{
		m_Movement.x += movement.x;
		m_Movement.y += movement.y;
	}

	inline void AddAcceleration(const XMFLOAT2& acc) noexcept
	{
		m_Acceleration.x += acc.x;
		m_Acceleration.y += acc.y;
	}

	constexpr void SetGravity(XMFLOAT2 g) noexcept { m_Gravity = g; }
	[[nodiscard]] constexpr auto GetGravity() const noexcept -> XMFLOAT2 { return m_Gravity; }

	constexpr void SetLevel(BBLevel* pLevel) noexcept { m_pLevel = pLevel; }
	[[nodiscard]] constexpr auto GetLevel() const noexcept -> BBLevel* { return m_pLevel; }
	
	constexpr void SetIsGrounded(bool val) noexcept { m_IsGrounded = val; }
	[[nodiscard]] constexpr auto IsGrounded() const noexcept -> bool { return m_IsGrounded; }

	constexpr void SetCollideDynamic(bool val) noexcept { m_CollidesDynamic = val; }
	[[nodiscard]] constexpr auto GetCollidesDynamic() const noexcept -> bool { return m_CollidesDynamic; }
	[[nodiscard]] constexpr auto GetTransform() const noexcept -> TransformComponent2D* { return m_pTransform; }
	[[nodiscard]] constexpr auto GetSize() const noexcept -> XMFLOAT2 { return m_Size; }

	void SetOnCollisionCB_X(std::function<void()> cb) { m_OnCollisionXCallback = cb; }
	void SetOnCollisionCB_Y(std::function<void()> cb) { m_OnCollisionYCallback = cb; }
	void SetOnDynamicCollisionCB(std::function<void(ECS::Entity*)> cb) { m_OnDynamicCollisionCallback = cb; m_CollidesDynamic = true; }

	void Update(float dt);

private:
	std::function<void()> m_OnCollisionXCallback{ []() {} };
	std::function<void()> m_OnCollisionYCallback{ []() {} };
	std::function<void(ECS::Entity*)> m_OnDynamicCollisionCallback{ [](ECS::Entity*) {} };

	TransformComponent2D* m_pTransform;
	ECS::System* m_pColliderSystem;
	XMFLOAT2 m_Movement;
	XMFLOAT2 m_Acceleration;
	bool m_IsGrounded = true;
	bool m_CollidesDynamic = false;

	XMFLOAT2 m_Size{ 60.f, 64.f };
	XMFLOAT2 m_Gravity{ 0.f, 781.f };
	XMFLOAT3 m_PreviousPosition;
	BBLevel* m_pLevel;

	bool m_MeetsRequirements;
};

#endif // !COLLIDER_COMPONENT_H