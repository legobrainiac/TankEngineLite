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

	static bool IsRunning;
	static BBLevel* pCurrentLevel;

private:
	void CameraTransitions(float dt);

	ECS::World* m_pWorld;
	
	SpriteBatch* m_pStatic_SB;
	SpriteBatch* m_pDynamic_SB;
	
	//////////////////////////////////////////////////////////////////////////
	// Game camera
	TransformComponent* m_pCameraTransform;
	XMFLOAT3 m_IntendedPosition;
	float m_CurrentScale = 0.f;
	float m_IntendedScale;
	bool m_IsDocked = false;
	bool m_Playing = false;

	// Keep track of players for connect/disconnect
	ECS::Entity* m_pPlayers[4]{ nullptr, nullptr, nullptr, nullptr };
};

#endif // !MAIN_GAME_H

