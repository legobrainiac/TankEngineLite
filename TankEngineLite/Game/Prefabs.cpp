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
