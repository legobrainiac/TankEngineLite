#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Tel.h"

class SpriteBatch;
class Entity;
class World;

class MainGame
	: public Game
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;
	void Shutdown() override;

private:
	ECS::World* m_pWorld;
	SpriteBatch* m_pCharacter_SB;
	SpriteBatch* m_pBackgroundStatic_SB;

	// TODO(tomas): move all of this to the engine
	// Tools
	bool m_DebugSystems = true;
	bool m_DebugRenderer = true;

	// Keep track of players for connect/disconnect
	ECS::Entity* m_pPlayers[4];
};

#endif // !MAIN_GAME_H

