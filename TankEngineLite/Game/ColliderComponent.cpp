#include "ColliderComponent.h"
#include "MainGame.h"
#include "BBLevel.h"

void ColliderComponent::Update(float dt)
{
	m_PreviousPosition = m_pTransform->position;
	
	//////////////////////////////////////////////////////////////////////////
	// Gravity x
	m_pTransform->position.x += m_Gravity.x * dt;
	m_pTransform->position.x += m_Movement.x * dt;
	m_pTransform->position.x += m_Acceleration.x * dt;

	if (MainGame::pCurrentLevel->IsOverlapping(
		{ m_pTransform->position.x, m_pTransform->position.y },
		{ m_pTransform->position.x + m_Size.x, m_pTransform->position.y + m_Size.y }
	))
	{
		m_Acceleration.x /= 2.f;
		m_pTransform->position.x = m_PreviousPosition.x;
		m_OnCollisionXCallback();
	}

	//////////////////////////////////////////////////////////////////////////
	// Gravity y
	uint8_t behaviour = 0U;
	float y = (m_Gravity.y * dt) + (m_Movement.y * dt) + (m_Acceleration.y * dt);
	m_pTransform->position.y += y;

	if (MainGame::pCurrentLevel->IsOverlapping(
		{ m_pTransform->position.x, m_pTransform->position.y },
		{ m_pTransform->position.x + m_Size.x, m_pTransform->position.y + m_Size.y }, 
		&behaviour))
	{
		if (behaviour == 2U)
		{
			// Jumping through a platform
			if (!MainGame::pCurrentLevel->IsOverlapping(
				{ m_PreviousPosition.x, m_PreviousPosition.y },
				{ m_PreviousPosition.x + m_Size.x, m_PreviousPosition.y + m_Size.y }))
			{
				if (m_PreviousPosition.y - m_pTransform->position.y < 0.f)
				{
					m_IsGrounded = true;
					m_pTransform->position.y = m_PreviousPosition.y;
					m_OnCollisionYCallback();
				}
			}
		}
		else
		{
			// Falling on to a platform
			if (m_PreviousPosition.y - m_pTransform->position.y <= 0.f)
				m_IsGrounded = true;

			m_Acceleration.y /= 2.f;
			m_pTransform->position.y = m_PreviousPosition.y;
			m_OnCollisionYCallback();
		}
	}
	else
	{
		// Falling
		if (m_PreviousPosition.y - m_pTransform->position.y <= 0.f)
			m_IsGrounded = false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Friction
	m_Acceleration.x = Utils::Lerp(m_Acceleration.x, 0.f, dt * 2.f);
	m_Acceleration.y = Utils::Lerp(m_Acceleration.y, 0.f, dt * 2.f);
	m_Movement = {};

	//////////////////////////////////////////////////////////////////////////
	// Clip world
	if (m_pTransform->position.y > 900.f)
	{
		m_pTransform->position.y = -30.f;
		m_Acceleration.x = 0.f;
		m_IsGrounded = false;
	}

	if (m_pTransform->position.y < -32.f)
	{
		m_pTransform->position.y = 900.f;
		m_Acceleration.x = 0.f;
		m_IsGrounded = false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Dynamic
	if (!m_CollidesDynamic)
		return;

	m_pColliderSystem->ForAll([this](ECS::EntityComponent* pC)
		{
			if (this == pC) 
				return;

			const auto col = static_cast<ColliderComponent*>(pC);
			auto otherTransform = col->GetTransform();

			float x1 = otherTransform->position.x;
			float y1 = otherTransform->position.y;
			float x2 = otherTransform->position.x + col->GetSize().x;
			float y2 = otherTransform->position.y + col->GetSize().y;

			if (m_pTransform->position.x > x2 || x1 > m_pTransform->position.x + m_Size.x || m_pTransform->position.y > y2 || y1 > m_pTransform->position.y + m_Size.y)
				return;
				
			m_OnDynamicCollisionCallback(pC->GetOwner());
		});
}