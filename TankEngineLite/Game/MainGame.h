#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Tel.h"

class SpriteBatch;
class BBLevel;
class Entity;
class Sound;
class World;

namespace FMOD
{
class Channel;
}

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
	static int aliveEnemyCount;
	static int alivePlayerCount;

	void LoadLevel(const std::string& level, bool score = false);

private:
	void CameraTransitions(float dt);

	enum GameState
	{
		MENU,
		START_GAME,
		CAMERA_ZOOM,
		END_NEXT,
		END_DEAD,
		LOAD_NEXT_LEVEL,
		PLAYING,
		DEAD
	} m_State = MENU;

	ECS::World* m_pWorld = nullptr;
	
	SpriteBatch* m_pStatic_SB = nullptr;
	SpriteBatch* m_pDynamic_SB = nullptr;
	SpriteBatch* m_pDynamicNumbers_SB = nullptr;
	Sound* m_pOST = nullptr;
	FMOD::Channel* m_pChannel = nullptr;
	
	//////////////////////////////////////////////////////////////////////////
	// Game camera
	TransformComponent* m_pCameraTransform = nullptr;
	XMFLOAT3 m_IntendedPosition{};
	float m_CurrentScale = 0.f;
	float m_IntendedScale;
	float m_Timer = 0.f;
	bool m_IsDocked = false;
	int m_MapIndex = 0;
	uint32_t m_NextControllerIndex = 0;

	std::string m_Maps[3] {"fire2", "fire", "wiki"};

	// Keep track of players for connect/disconnect
	ECS::Entity* m_pPlayers[4]{ nullptr, nullptr, nullptr, nullptr };
};

#endif // !MAIN_GAME_H

