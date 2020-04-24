#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Tel.h"

class SpriteBatch;
class Entity;
class World;

class TransformComponent2D;

class MainGame
	: public Game
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager, TEngineRunner* pEngine) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;
	void Shutdown() override;

private:
	ECS::World* m_pWorld;
	SpriteBatch* m_pStatic_SB;
	SpriteBatch* m_pDynamic_SB;
	
	TransformComponent2D* m_pParticleEmitterTransform;

	// Keep track of players for connect/disconnect
	ECS::Entity* m_pPlayers[4];
};

#endif // !MAIN_GAME_H

