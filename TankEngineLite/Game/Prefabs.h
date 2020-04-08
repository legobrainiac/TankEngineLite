#ifndef PREFABS_H
#define PREFABS_H

#include "BasicComponents.h"
#include "InputManager.h"

class SpriteBatch;

class Prefabs
{
public:
	static ECS::Entity* CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch);
	static ECS::Entity* CreatePlayer(ECS::World* pWorld, SpriteBatch* pSpriteBatch, XMFLOAT2 pos, Player player);
};

#endif 