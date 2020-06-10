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

		if (m_pTransform)
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

	constexpr void SetLevel(BBLevel* pLevel) noexcept { m_pLevel = pLevel; }
	[[nodiscard]] constexpr auto GetLevel() const noexcept -> BBLevel* { return m_pLevel; }
	
	constexpr void SetIsGrounded(bool val) noexcept { m_IsGrounded = val; }
	[[nodiscard]] constexpr auto IsGrounded() const noexcept -> bool { return m_IsGrounded; }

	void SetOnCollisionCB_X(std::function<void()> cb) { m_OnCollisionXCallback = cb; }
	void SetOnCollisionCB_Y(std::function<void()> cb) { m_OnCollisionYCallback = cb; }

	void Update(float dt);

private:
	std::function<void()> m_OnCollisionXCallback{ []() {} };
	std::function<void()> m_OnCollisionYCallback{ []() {} };

	TransformComponent2D* m_pTransform;
	XMFLOAT2 m_Movement;
	XMFLOAT2 m_Acceleration;
	bool m_IsGrounded = true;

	XMFLOAT2 m_Size{ 60.f, 64.f };
	XMFLOAT2 m_Gravity{ 0.f, 781.f };
	XMFLOAT3 m_PreviousPosition;
	BBLevel* m_pLevel;

	bool m_MeetsRequirements;
};

#endif // !COLLIDER_COMPONENT_H