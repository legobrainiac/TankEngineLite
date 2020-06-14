#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

#include "Model.h"

#include "Profiler.h"
#include "BinaryInterfaces.h"

#include "Sound.h"
#include "SoundManager.h"

#include "MaitaController.h"
#include "PlayerController.h"
#include "ColliderComponent.h"
#include "ZenChanController.h"

#include "BBLevel.h"

#define TINY	8
#define SMALL	256
#define MED		1024
#define BIG		4096

bool MainGame::IsRunning = false;
BBLevel* MainGame::pCurrentLevel = nullptr;
int MainGame::aliveEnemyCount = 0;
int MainGame::alivePlayerCount = 0;

void MainGame::Initialize()
{
	using namespace ECS;

	// Initialized world and world systems
	m_pWorld = Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		WorldSystem<TransformComponent2D, SMALL, 0, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<SpriteRenderComponent, SMALL, 1, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<LifeSpan, SMALL, 2, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ProjectileComponent, SMALL, 3, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ColliderComponent, SMALL, 4, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<PlayerController, TINY, 5, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ParticleEmitter, TINY, 6, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<Particle, MED, 7, ExecutionStyle::ASYNCHRONOUS>,
		WorldSystem<TransformComponent, TINY, 8, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<CameraComponent, TINY, 9, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ModelRenderComponent, TINY, 10, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ZenChanController, TINY, 11, ExecutionStyle::ASYNCHRONOUS>,
		WorldSystem<MaitaController, TINY, 12, ExecutionStyle::ASYNCHRONOUS>
	>();

	// Initialize custom resource loaders
	ResourceManager::AddTypeResolver(
		std::pair(".bmap", [](std::string path, std::string name) { RESOURCES->Load<BBLevel>(path, name); })
	);
}

void MainGame::Load([[maybe_unused]] ResourceManager* pResourceManager, [[maybe_unused]] TEngineRunner* pEngine)
{
	//////////////////////////////////////////////////////////////////////////
	// Create the dynamic sprite batch
	m_pDynamic_SB = new (Memory::New<SpriteBatch>()) SpriteBatch("Dynamic");
	m_pDynamic_SB->InitializeBatch(RESOURCES->Get<Texture>("atlas_0"));
	pEngine->RegisterBatch(m_pDynamic_SB);

	m_pDynamicNumbers_SB = new (Memory::New<SpriteBatch>()) SpriteBatch("Dynamic Number");
	m_pDynamicNumbers_SB->InitializeBatch(RESOURCES->Get<Texture>("atlas_9"));
	pEngine->RegisterBatch(m_pDynamicNumbers_SB);

	//////////////////////////////////////////////////////////////////////////
	// Create the static sprite batch
	m_pStatic_SB = new(Memory::New<SpriteBatch>()) SpriteBatch("Static");
	m_pStatic_SB->InitializeBatch(RESOURCES->Get<Texture>("atlas_7"), BatchMode::BATCHMODE_STATIC);
	pEngine->RegisterBatch(m_pStatic_SB);


	//////////////////////////////////////////////////////////////////////////
	// Set camera for sprite batches
	m_pDynamic_SB->SetCamera({ -320.f, 0.f });
	m_pStatic_SB->SetCamera({ -320.f, 0.f });

	m_pDynamic_SB->SetScale(0.0f);
	m_pStatic_SB->SetScale(0.0f);

	//////////////////////////////////////////////////////////////////////////
	// Load level
	m_pOST = RESOURCES->Get<Sound>("mainTheme");
	m_pChannel = SOUND->Play(m_pOST->GetSound());
	m_pChannel->setPaused(true);
	m_pChannel->setVolume(0.25f);

	//////////////////////////////////////////////////////////////////////////
	// Create 3D camera
	const auto pCamera = Prefabs::SpawnCamera(m_pWorld, { 0.f, -5.f, 20.f });
	m_pCameraTransform = pCamera->GetComponent<TransformComponent>();
	m_IntendedPosition = { 0.f, -5.f, 20.f };

	//////////////////////////////////////////////////////////////////////////
	// Create world model
	{
		const auto pModel = RESOURCES->Get<Model>("arcade");
		const auto pTexture = RESOURCES->Get<Texture>("arcade_diffuse_pog");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(0.f, XM_PIDIV2, 0.f);
		pTransform->scale = { 0.01f, 0.01f, 0.01f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Create sky dome model
	{
		const auto pModel = RESOURCES->Get<Model>("skydome");
		const auto pTexture = RESOURCES->Get<Texture>("skydome_diffuse");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(0.f, XM_PIDIV2, 0.f);
		pTransform->scale = { 0.05f, 0.05f, 0.05f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Player spawning
	InputManager::GetInstance()->RegisterControllerConnectedCallback(
		[this](uint32_t controller, ConnectionType connection)
		{
			if (connection == ConnectionType::CONNECTED)
			{
				if (controller >= m_NextControllerIndex)
				{
					m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { 84, 900 - 144 }, (Player)controller);
					m_NextControllerIndex++;
				}
			}
		});

	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_P, ActionType::PRESSED,
			[this]()
			{
				if (m_NextControllerIndex >= 2)
					return;

				uint32_t controller = m_NextControllerIndex++;
				MainGame::alivePlayerCount++;
				m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { 84, 900 - 144 }, (Player)controller);
			}));

	//////////////////////////////////////////////////////////////////////////
	// Check for new controllers
	InputManager::GetInstance()->CheckControllerConnection();
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_T, ActionType::PRESSED,
			[]()
			{
				InputManager::GetInstance()->CheckControllerConnection();
			}));

	if (m_NextControllerIndex == 0)
	{
		uint32_t controller = m_NextControllerIndex++;
		MainGame::alivePlayerCount++;
		m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { 84, 900 - 144 }, (Player)controller);
	}

	//////////////////////////////////////////////////////////////////////////
	// Start play
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_RETURN, ActionType::PRESSED,
			[this]()
			{
				m_State = CAMERA_ZOOM;
			}));

	LoadLevel("fire2");
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	switch (m_State)
	{
	case MENU:

		break;
	case START_GAME:
		m_State = PLAYING;
		break;
	case CAMERA_ZOOM:
		CameraTransitions(dt);
		break;
	case END_DEAD:
		m_Timer += dt;

		if (m_Timer > 10.f)
		{
			m_Timer = 0;
			m_State = PLAYING;

			m_pWorld->MessageAll(128U);

			m_MapIndex = 0;
			LoadLevel(m_Maps[m_MapIndex]);
			m_State = PLAYING;
		}
		break;
	case END_NEXT:
		m_Timer += dt;

		if (m_Timer > 5.f)
		{
			m_Timer = 0;
			m_State = LOAD_NEXT_LEVEL;
		}
		break;
	case LOAD_NEXT_LEVEL:
		m_MapIndex++;
		if (m_MapIndex > 2)
			m_MapIndex = 0;

		LoadLevel(m_Maps[m_MapIndex]);
		m_State = PLAYING;
		break;
	case PLAYING:
	{
		if (MainGame::aliveEnemyCount == 0)
			m_State = END_NEXT;

		if (MainGame::alivePlayerCount == 0)
			m_State = END_DEAD;

		const auto DrawNumbers = [this](uint32_t number, DirectX::XMFLOAT2 position)
		{
			constexpr auto offset = 48U;
			constexpr auto ssOffset = 64.f;
			const auto numberString = std::to_string(number);

			for (size_t i = 0; i < numberString.size(); ++i)
			{
				int ni = numberString[i] - offset;

				XMFLOAT4 rect{ni * 16.f, 0.f, (ni + 1) * 16.f, 16 };

				m_pDynamicNumbers_SB->PushSprite(rect, { position.x, position.y, 0 }, 0.f, { 4.f, 4.f }, { 0.f, 0.f }, { 1.f, 1.f, 1.f, 1.f });
				position.x += ssOffset;
			}
		};

		for (int i = 0; i < 2; ++i)
		{
			float offset = -300.f;
			float scoreOffset = 0.f;
			if (i == 1)
			{
				offset = 1000.f;
				scoreOffset = 1300.f;
			}

			if (m_pPlayers[i])
			{
				auto player = m_pPlayers[i]->GetComponent<PlayerController>();
				int health = player->GetHealth();

				for (int h = 0; h < health; ++h)
					m_pDynamic_SB->PushSprite({ 128 - 16, 128, 144 - 16, 144 }, { offset + h * 64, 0, 0 }, 0.f, { 4.f, 4.f }, { 0.f, 0.f }, { 1.f, 1.f, 1.f, 1.f });
			
				DrawNumbers(player->GetScore(), { scoreOffset, 100.f });
			}
		}
	}
	break;
	case DEAD:

		break;
	}

	m_CurrentScale = Utils::Lerp(m_CurrentScale, m_IntendedScale, dt * 4);
	m_pDynamic_SB->SetScale(m_CurrentScale);
	m_pStatic_SB->SetScale(m_CurrentScale);

	PROFILE(SESSION_UPDATE_ECS, ECS::Universe::GetInstance()->Update(dt));
}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	// Populate sprite batches
	PROFILE(SESSION_RENDER_ECS, pRenderer->SpriteBatchRender(m_pWorld->GetSystemByComponent<SpriteRenderComponent>()));
	PROFILE(SESSION_RENDER_ECS, pRenderer->ModelRender(m_pWorld->GetSystemByComponent<ModelRenderComponent>()));

	// Render your sprite batches
	Profiler::GetInstance()->BeginSubSession<SESSION_BATCH_RENDERING>();
	PROFILE(SESSION_BATCH_DYNAMIC, m_pDynamic_SB->Render());
	PROFILE(SESSION_BATCH_DYNAMIC, m_pDynamicNumbers_SB->Render());
	PROFILE(SESSION_BATCH_STATIC, m_pStatic_SB->Render());
	Profiler::GetInstance()->EndSubSession();
}

void MainGame::Shutdown()
{
	m_pDynamic_SB->Destroy();
	Memory::Delete(m_pDynamic_SB);

	m_pDynamicNumbers_SB->Destroy();
	Memory::Delete(m_pDynamicNumbers_SB);

	m_pStatic_SB->Destroy();
	Memory::Delete(m_pStatic_SB);
}

void MainGame::LoadLevel(const std::string& level, bool score)
{
	MainGame::aliveEnemyCount = 0;
	MainGame::alivePlayerCount = 0;

	pCurrentLevel = RESOURCES->Get<BBLevel>(level);
	pCurrentLevel->SetupBatch(m_pStatic_SB);

	//////////////////////////////////////////////////////////////////////////
	std::array<TransformComponent2D*, 4> playerTransforms{};

	for (uint32_t i = 0U; i < 4U; ++i)
	{
		if (m_pPlayers[i])
			playerTransforms[i] = m_pPlayers[i]->GetComponent<TransformComponent2D>();
	}

	// Spawn enemies
	for (int i = 0; i < 4; ++i)
	{
		int index = MainGame::pCurrentLevel->m_Footer.zenSpawns[i];

		if (index == 0)
			continue;

		MainGame::aliveEnemyCount++;

		XMFLOAT3 pos
		{
			(float)(index % MainGame::pCurrentLevel->m_Header.mapW) * (MainGame::pCurrentLevel->m_Header.tileW * 2.f),
			(float)((index - MainGame::pCurrentLevel->m_Header.mapW) / MainGame::pCurrentLevel->m_Header.mapW) * (MainGame::pCurrentLevel->m_Header.tileH * 2.f),
			0.1f
		};

		pos.y -= 34.f;
		pos.x -= 30.f;

		Prefabs::CreateZenChan(m_pWorld, m_pDynamic_SB, playerTransforms, pos);
	}

	for (int i = 0; i < 4; ++i)
	{
		int index = MainGame::pCurrentLevel->m_Footer.maitaSpawns[i];

		if (index == 0)
			continue;

		MainGame::aliveEnemyCount++;

		XMFLOAT3 pos
		{
			(float)(index % MainGame::pCurrentLevel->m_Header.mapW) * (MainGame::pCurrentLevel->m_Header.tileW * 2.f),
			(float)((index - MainGame::pCurrentLevel->m_Header.mapW) / MainGame::pCurrentLevel->m_Header.mapW) * (MainGame::pCurrentLevel->m_Header.tileH * 2.f),
			0.1f
		};

		pos.y -= 34.f;
		pos.x -= 30.f;

		Prefabs::CreateMaita(m_pWorld, m_pDynamic_SB, playerTransforms, pos);
	}

	for (int i = 0; i < 4; ++i)
	{
		if (m_pPlayers[i])
		{
			m_pPlayers[i]->GetComponent<TransformComponent2D>()->position = { 84, 900 - 144, 0.f };
			m_pPlayers[i]->GetComponent<PlayerController>()->Reset(score);
			MainGame::alivePlayerCount++;
		}
	}
}

void MainGame::CameraTransitions(float dt)
{
	static float timer = 0.f;

	timer += dt;
	m_IntendedPosition = { 0.f, -5.f, 0.f };

	if (timer > 3.f)
	{
		m_IntendedScale = 1.f;
		MainGame::IsRunning = true;
		m_State = PLAYING;
		m_pChannel->setPaused(false);
	}

	float cameraInterpDT = dt;
	XMStoreFloat3(&m_pCameraTransform->position, XMVectorLerp(XMLoadFloat3(&m_pCameraTransform->position), XMLoadFloat3(&m_IntendedPosition), cameraInterpDT));
	m_CurrentScale = Utils::Lerp(m_CurrentScale, m_IntendedScale, dt * 4);
	m_pDynamic_SB->SetScale(m_CurrentScale);
	m_pStatic_SB->SetScale(m_CurrentScale);
}
