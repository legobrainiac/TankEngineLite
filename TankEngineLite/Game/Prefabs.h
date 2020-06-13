#ifndef PREFABS_H
#define PREFABS_H

#include "BasicComponents.h"
#include "InputManager.h"

class BBLevel;
class SpriteBatch;

class Prefabs
{
public:
	static ECS::Entity* CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch);
	static ECS::Entity* SpawnCamera(ECS::World* pWorld, XMFLOAT3 pos);
	static ECS::Entity* SpawnParticle(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startingAcceleration, XMFLOAT4 colour, float life, float gravity);
	
	enum ScoreType
	{
		PIZZA,
		WATERMELON
	};
	static ECS::Entity* SpawnScoreItem(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, ScoreType type);
	static ECS::Entity* SpawnScore(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, ScoreType type);
	static ECS::Entity* ShootBolder(ECS::World* pWorld, SpriteBatch* pSpriteBatch, const XMFLOAT2& pos, const XMFLOAT2& direction);

	static ECS::Entity* CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player);
	static ECS::Entity* CreateZenChan(ECS::World* pWorld, SpriteBatch* pSpriteBatch, const std::array<TransformComponent2D*, 4>& playerTransforms, XMFLOAT3 pos);
	static ECS::Entity* CreateMaita(ECS::World* pWorld, SpriteBatch* pSpriteBatch, const std::array<TransformComponent2D*, 4>& playerTransforms, XMFLOAT3 pos);
};

#endif 