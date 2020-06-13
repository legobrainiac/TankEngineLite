#include "ZenChanController.h"
#include "MainGame.h"
#include "Prefabs.h"

ZenChanController::ZenChanController(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_MeetsRequirements = false;

	m_pTransform = pE->GetComponent<TransformComponent2D>();
	m_pRenderComponent = pE->GetComponent<SpriteRenderComponent>();
	m_pCollider = pE->GetComponent<ColliderComponent>();

	if (m_pTransform != nullptr && m_pRenderComponent != nullptr && m_pCollider != nullptr)
	{
		m_pCollider->SetOnCollisionCB_X([this]()
			{
				m_FacingRight = !m_FacingRight;
			});

		m_pCollider->SetOnDynamicCollisionCB([](ECS::Entity* pE)
			{
				if (pE->GetTag() == 69U)
					pE->Message(0U);
			});

		m_MeetsRequirements = true;
	}
}

void ZenChanController::Update(float dt)
{
	if (!m_MeetsRequirements || !MainGame::IsRunning)
		return;

	int tileMultiplier = 2;
	m_SpriteTimer += dt;
	switch (m_State)
	{
	case ZCStates::NORMAL:
	{
		tileMultiplier = 2;
		m_pCollider->SetFixedMovement({ (m_FacingRight) ? m_MovingSpeed : -m_MovingSpeed, 781.f });

		// Jump towards closest player
		float closest = FLT_MAX;
		TransformComponent2D* pClosestPlayer = nullptr;
		for (uint32_t i = 0U; i < 4U; ++i)
		{
			if (m_Players[i])
			{
				auto transform = m_Players[i];
				float distance = fabs(m_pTransform->position.y - transform->position.y);
				if (distance < closest)
				{
					closest = distance;
					pClosestPlayer = transform;
				}
			}
		}

		if ((pClosestPlayer->position.y + 128) < m_pTransform->position.y && m_pCollider->IsGrounded() && closest < 500.f)
		{
			m_JumpTimer += dt;
			if (m_JumpTimer > 0.5f)
			{
				m_pCollider->AddAcceleration({ 0.f, -1750.f });
				m_pCollider->SetIsGrounded(false);
				m_JumpTimer = 0.f;
			}
		}
	}
	break;
	case ZCStates::IN_BUBBLE:
		tileMultiplier = 15;
		m_FacingRight = true;
		m_BubbleTimer += dt;

		m_pCollider->SetFixedMovement({ sin(m_BubbleTimer) * 50.f, -81.f });

		if (m_BubbleTimer > 5.f)
		{
			m_BubbleTimer = 0;
			m_State = ZCStates::NORMAL;

			// Reset callback
			m_pCollider->SetOnDynamicCollisionCB([](ECS::Entity* pE)
				{
					if (pE->GetTag() == 69U)
						pE->Message(0U);
				});
		}
		break;
	}

	// Animations
	// TODO(tomas): move this to it's own component
	if (m_SpriteTimer > 0.1f)
	{
		// Sprite animation right
		constexpr XMFLOAT4 atlastTransformsRight[8]
		{
			{ 0, 0, 16, 16 },
			{ 16, 0, 32, 16 },
			{ 32, 0, 48, 16 },
			{ 48, 0, 64, 16 },
			{ 64, 0, 80, 16 },
			{ 80, 0, 96, 16 },
			{ 96, 0, 112, 16 },
			{ 112, 0, 128, 16 }
		};

		auto atlasTransform = atlastTransformsRight[m_SpriteIndex];
		atlasTransform.y += tileMultiplier * 16;
		atlasTransform.w += tileMultiplier * 16;

		m_SpriteTimer -= m_SpriteTimer;
		m_SpriteIndex++;

		if (m_SpriteIndex == 8)
			m_SpriteIndex = 0;

		// Which of the sprite should you use
		if (!m_FacingRight)
		{
			atlasTransform.x += 128;
			atlasTransform.z += 128;
		}

		m_pRenderComponent->SetAtlasTransform(atlasTransform);
	}
}

void ZenChanController::OnMessage(uint32_t message)
{
	if (message == 128U)
		m_pOwner->GetWorld()->AsyncDestroyEntity(m_pOwner->GetId());

	if (message == 0U && m_State == ZCStates::NORMAL)
	{
		m_State = ZCStates::IN_BUBBLE;
		Prefabs::SpawnScore(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), { m_pTransform->position.x, m_pTransform->position.y }, Prefabs::WATERMELON);

		m_pCollider->SetOnDynamicCollisionCB([this](ECS::Entity* pE)
			{
				if (pE->GetTag() == 69U)
				{
					MainGame::aliveEnemyCount--;
					pE->Message(1U);
					m_pOwner->GetWorld()->AsyncDestroyEntity(m_pOwner->GetId());

					auto pos = m_pTransform->position;
					pos.x += 32.f;
					pos.y += 32.f;

					for (uint32_t i = 0; i < 10; ++i)
					{
						float angle = (float)Utils::RandInterval(0, 360) * (float)M_PI / 180.f;
						XMFLOAT2 accel{ cosf(angle) * 100.f, sin(angle) * 100.f };
						Prefabs::SpawnParticle(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), { pos.x, pos.y }, accel, { 0.0f, 1.f, 0.f, 1.f }, 0.5f, 50.f);
					}
					Prefabs::SpawnScoreItem(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), { pos.x - 32.f, pos.y - 32.f }, Prefabs::WATERMELON);
					Prefabs::SpawnScore(m_pOwner->GetWorld(), m_pRenderComponent->GetSpriteBatch(), { pos.x - 32.f, pos.y - 32.f }, Prefabs::WATERMELON);
				}
			});
	}
}
