#ifndef PREFABS_H
#define PREFABS_H

#include "BasicComponents.h"

class SpriteBatch;

class Prefabs
{
public:
	static ECS::Entity* CreateBubbleProjectile(ECS::World* pWorld, const XMFLOAT2& position, const XMFLOAT2& direction, SpriteBatch* pSpriteBatch);
};

#endif 