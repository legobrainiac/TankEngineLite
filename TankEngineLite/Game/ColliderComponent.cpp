#include "ColliderComponent.h"
#include "BBLevel.h"

void ColliderComponent::Update(float dt)
{
	m_PreviousPosition = m_pTransform->position;
	
	//////////////////////////////////////////////////////////////////////////
	// Gravity x
	m_pTransform->position.x += m_Gravity.x * dt;
	m_pTransform->position.x += m_Movement.x * dt;
	m_pTransform->position.x += m_Acceleration.x * dt;

	if (m_pLevel->IsOverlapping(
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
	float y = (m_Gravity.y * dt) + (m_Movement.y * dt) + (m_Acceleration.y * dt);
	m_pTransform->position.y += y;

	if (m_pLevel->IsOverlapping(
		{ m_pTransform->position.x, m_pTransform->position.y },
		{ m_pTransform->position.x + m_Size.x, m_pTransform->position.y + m_Size.y }
	))
	{
		if (m_PreviousPosition.y - m_pTransform->position.y <= 0.f)
			m_IsGrounded = true;
		
		m_Acceleration.y /= 2.f;
		m_pTransform->position.y = m_PreviousPosition.y;
		m_OnCollisionYCallback();
	}

	//////////////////////////////////////////////////////////////////////////
	// Friction
	m_Acceleration.x = Utils::Lerp(m_Acceleration.x, 0.f, dt * 2.f);
	m_Acceleration.y = Utils::Lerp(m_Acceleration.y, 0.f, dt * 2.f);
	m_Movement = {};
}