#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "BasicComponents.h"
#include "Tel.h"
#include "ecs.h"

class SpriteBatch;

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
};

#endif // !MAIN_GAME_H

