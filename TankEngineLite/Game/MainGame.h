#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Tel.h"

class SpriteBatch;
class BBLevel;
class Entity;
class World;

class TransformComponent2D;

class MainGame
	: public IGame
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager, TEngineRunner* pEngine) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;
	void Shutdown() override;

private:
	void CameraTransitions(float dt);

	ECS::World* m_pWorld;
	BBLevel* m_pCurrentLevel;
	SpriteBatch* m_pStatic_SB;
	SpriteBatch* m_pDynamic_SB;
	
	//////////////////////////////////////////////////////////////////////////
	// Game camera
	CameraComponent* m_pCamera;
	TransformComponent* m_pCameraTransform;
	XMFLOAT3 m_IntendedPosition;
	bool m_IsDocked = true;
	bool m_Animating = false;

	// Keep track of players for connect/disconnect
	ECS::Entity* m_pPlayers[4];
};

#endif // !MAIN_GAME_H

