#include "pch.h"
#include "Prefabs.h"
#include "ColliderComponent.h"
#include "PlayerController.h"
#include "BBLevel.h"

ECS::Entity* Prefabs::CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch, BBLevel* pLevel)
{
	auto pEntity = pWorld->CreateEntity();
	auto [pProjectile, pCollider, pLifeSpan, pRenderer, pTransform] = pEntity->PushComponents<ProjectileComponent, ColliderComponent, LifeSpan, SpriteRenderComponent, TransformComponent2D>();

	// Component setup
	pTransform->position = { position.x, position.y, 0.1f };
	pTransform->scale = { 4.f, 4.f };

	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 224,  16, 240 });
	pRenderer->SetPivot({});

	pLifeSpan->SetLifeSpan(5.f);

	pProjectile->SetDirection(direction);
	pProjectile->SetSpeed(1000.f);

	pCollider->SetLevel(pLevel);
	pCollider->SetOnDynamicCollisionCB([]([[maybe_unused]] ECS::Entity* pOther) 
		{
			if (pOther->GetTag() != 69U)
				pOther->GetWorld()->AsyncDestroyEntity(pOther->GetId());
		});
	
	return pEntity;
}

ECS::Entity* Prefabs::CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player, BBLevel* pLevel)
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
	pCollider->SetLevel(pLevel);

	pEntity->SetTag(69U);

	return pEntity;
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
