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
	static ECS::Entity* CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player, BBLevel* pLevel);
	static ECS::Entity* SpawnParticle(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, XMFLOAT2 startingAcceleration, XMFLOAT4 colour, float life, float gravity);
};

#endif 