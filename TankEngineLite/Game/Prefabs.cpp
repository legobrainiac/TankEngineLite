#include "pch.h"
#include "Prefabs.h"

ECS::Entity* Prefabs::CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch)
{
	auto pEntity = pWorld->CreateEntity();
	auto [pLifeSpan, pRenderer, pProjectile, pTransform] = pEntity->PushComponents<LifeSpan, SpriteRenderComponent, ProjectileComponent, TransformComponent2D>();

	// Component setup
	pTransform->position = { position.x, position.y, 0.1f };
	pTransform->scale = { 4.f, 4.f };

	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 224,  16, 240 });

	pLifeSpan->SetLifeSpan(2.f);

	pProjectile->SetDirection(direction);
	pProjectile->SetSpeed(1000.f);

	return pEntity;
}

ECS::Entity* Prefabs::CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player)
{
	auto pEntity = pWorld->CreateEntity();
	auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<PlayerController, SpriteRenderComponent, TransformComponent2D>();

	// Component setup
	pRenderer->SetSpriteBatch(pSpriteBatch);
	pRenderer->SetAtlasTransform({ 0, 0, 16, 16 });

	pTransform->position = { pos.x, pos.y, 0.f };
	pTransform->scale = { 4.f, 4.f };

	pMovement->SetInputController(player);

	return pEntity;
}

ECS::Entity* Prefabs::SpawnParticle(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startingAcceleration, XMFLOAT4 colour, float life, float gravity)
{
	pWorld->AsyncCreateEntity(
		[pSpriteBatch, pos, startingAcceleration, colour, life, gravity](ECS::Entity* pEntity) 
		{
			auto [pParticle] = pEntity->PushComponents<Particle>();
			pParticle->Initialize(pSpriteBatch, pos, startingAcceleration, 0.25f, colour, life, gravity);
			return pEntity;
		}
	);

	return nullptr;
}
