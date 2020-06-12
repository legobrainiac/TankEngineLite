#include "pch.h"
#include "Prefabs.h"
#include "ZenChanController.h"
#include "ColliderComponent.h"
#include "PlayerController.h"
#include "CoreComponents.h"
#include "Renderer.h"
#include "BBLevel.h"
#include "MainGame.h"

ECS::Entity* Prefabs::CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch)
{
	auto pEntity = pWorld->CreateEntity();
	auto [pProjectile, pCollider, pLifeSpan, pRenderer, pTransform] = pEntity->PushComponents<ProjectileComponent, ColliderComponent, LifeSpan, SpriteRenderComponent, TransformComponent2D>();

	// Component setup
	pTransform->position = { position.x, position.y, 0.1f };
	pTransform->scale = { 1.f, 1.f };

	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 224,  16, 240 });

	pLifeSpan->SetLifeSpan(5.f);

	pProjectile->SetDirection(direction);
	pProjectile->SetSpeed(1000.f);

	pCollider->SetFixedMovement({ 0.f, 0.f });
	pCollider->SetSize({ 16.f, 16.f });
	pCollider->SetOnDynamicCollisionCB([pEntity, pWorld]([[maybe_unused]] ECS::Entity* pOther)
		{
			if (pOther->GetTag() == 32U)
			{
				pOther->Message(0U);
				pWorld->AsyncDestroyEntity(pEntity->GetId());
			}
		});

	pCollider->SetOnCollisionCB_X([pEntity, pWorld]()
		{ // TODO(tomas): spawn empty bubble, that functions like the projectile but behave like a full bubble
			pWorld->AsyncDestroyEntity(pEntity->GetId());
		});

	return pEntity;
}

ECS::Entity* Prefabs::CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player)
{
	auto pEntity = pWorld->CreateEntity();
	auto [pMovement, pCollider, pRenderer, pTransform] = pEntity->PushComponents<PlayerController, ColliderComponent, SpriteRenderComponent, TransformComponent2D>();

	// Component setup
	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 0, 16, 16 });
	pRenderer->SetPivot({});

	pTransform->position = { pos.x, pos.y, 0.f };
	pTransform->scale = { 4.f, 4.f };

	pMovement->SetInputController(player);

	pEntity->SetTag(69U);

	return pEntity;
}

ECS::Entity* Prefabs::CreateZenChan(ECS::World* pWorld, SpriteBatch* pSpriteBatch,  const std::array<TransformComponent2D*, 4> & playerTransforms, XMFLOAT3 pos)
{
	const auto ai = pWorld->CreateEntity();
	auto [pController, pCollider, pRenderer, pTransform] = ai->PushComponents<ZenChanController, ColliderComponent, SpriteRenderComponent, TransformComponent2D>();

	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 32, 16, 16 + 32 });
	pRenderer->SetPivot({});

	pTransform->position = { pos.x, pos.y, pos.z };
	pTransform->scale = { 4.f, 4.f };
	ai->SetTag(32U);

	pController->SetPlayers(playerTransforms);

	return ai;
}

ECS::Entity* Prefabs::SpawnCamera(ECS::World* pWorld, XMFLOAT3 pos)
{
	auto pCamera = pWorld->CreateEntity();
	auto [pCameraComponent, pTransform] = pCamera->PushComponents<CameraComponent, TransformComponent>();
	Renderer::GetInstance()->GetDirectX()->SetCamera(pCameraComponent);
	pTransform->Translate(pos);

	return pCamera;
}

ECS::Entity* Prefabs::SpawnParticle(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startingAcceleration, XMFLOAT4 colour, float life, float gravity)
{
	pWorld->AsyncCreateEntity(
		[pSpriteBatch, pos, startingAcceleration, colour, life, gravity](ECS::Entity* pEntity)
		{
			auto [pParticle] = pEntity->PushComponents<Particle>();
			pParticle->Initialize(pSpriteBatch, pos, startingAcceleration, 0.35f, colour, life, gravity);
			return pEntity;
		}
	);

	return nullptr;
}
