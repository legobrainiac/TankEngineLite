#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "BasicComponents.h"
#include "Tel.h"
#include "ecs.h"

class MainGame
	: public Game
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;

private:
	TextComponent* m_pFpsTextComponent;
	ECS::World* m_pWorld;
};

#endif // !MAIN_GAME_H

