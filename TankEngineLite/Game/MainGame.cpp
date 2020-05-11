#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

#include "Model.h"

#include "Profiler.h"

void MainGame::Initialize()
{
	using namespace ECS;

	// Initialized world and world systems
	m_pWorld = Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		WorldSystem<TransformComponent2D, 256, 0, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<SpriteRenderComponent, 256, 1, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<LifeSpan, 256, 2, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ProjectileComponent, 256, 3, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<PlayerController, 8, 4, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ParticleEmitter, 16, 5, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<Particle, 4096, 6, ExecutionStyle::ASYNCHRONOUS>,
		WorldSystem<TransformComponent, 8, 7, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<CameraComponent, 8, 8, ExecutionStyle::SYNCHRONOUS>
	>();
}

void MainGame::Load([[maybe_unused]] ResourceManager* pResourceManager, [[maybe_unused]] TEngineRunner* pEngine)
{
	//////////////////////////////////////////////////////////////////////////
	// Create the dynamic sprite batch
	m_pDynamic_SB = new (Memory::New<SpriteBatch>()) SpriteBatch("Dynamic");
	m_pDynamic_SB->InitializeBatch(RESOURCES->Get<Texture>("atlas_0"));
	pEngine->RegisterBatch(m_pDynamic_SB);
    
	//////////////////////////////////////////////////////////////////////////
	// Create the static sprite batch
	m_pStatic_SB = new(Memory::New<SpriteBatch>()) SpriteBatch("Static");
	m_pStatic_SB->InitializeBatch(RESOURCES->Get<Texture>("atlas_5"), BatchMode::BATCHMODE_STATIC);
	pEngine->RegisterBatch(m_pStatic_SB);

	// Create particle system
	ParticleEmitter* pParticleSystem = nullptr;
	{
		auto pParticleSystemEntity = m_pWorld->CreateEntity();
		auto[pParticleEmitter, pTransform] = pParticleSystemEntity->PushComponents<ParticleEmitter, TransformComponent2D>();
		pParticleEmitter->m_ParticleSpawnInterval = 0.001f;
		pParticleEmitter->m_ParticleLifeTime = .5f;
		pParticleEmitter->m_ParticlesPerSpawn = 5U;
		pParticleEmitter->m_pSpriteBatch = m_pDynamic_SB;
		pParticleEmitter->m_Gravity = 981.f;

		m_pParticleEmitterTransform = pTransform;
		pParticleSystem = pParticleEmitter;
	}

	// Setup a smol platform
	for (int i = 0; i < 25; ++i)
		m_pStatic_SB->PushSprite({ 0, 0, 16, 16 }, { (float)i * 64, 600, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });

	for (int j = 1; j < 5; ++j)
	{
		for (int i = 0; i < 25; ++i)
			m_pStatic_SB->PushSprite({ 16, 0, 32, 16 }, { (float)i * 64, 600 + (float)j * 64, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });
	}

	// If a controller is connected, 
	//  spawn a player for it
	// If a controller is disconnected
	//  destroy the corresponding player
	InputManager::GetInstance()->RegisterControllerConnectedCallback(
		[this](uint32_t controller, ConnectionType connection) 
		{
			if (connection == ConnectionType::CONNECTED)
				m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { float(rand() % 1000 + 300), 0 }, (Player)controller);
			else
				m_pWorld->DestroyEntity(m_pPlayers[controller]->GetId());
		});

	InputManager::GetInstance()->CheckControllerConnection();

	// Toggle particle emission
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_R, ActionType::PRESSED,
			[pParticleSystem]()
			{
				pParticleSystem->ToggleSpawning();
			}));

	// Check for new controllers
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_T, ActionType::PRESSED,
			[pParticleSystem]()
			{
				InputManager::GetInstance()->CheckControllerConnection();
			}));
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	PROFILE(SESSION_UPDATE_ECS, ECS::Universe::GetInstance()->Update(dt));

	auto [x, y, state] = pInputManager->GetMouseState();
	m_pParticleEmitterTransform->position.x = (float)x;
	m_pParticleEmitterTransform->position.y = (float)y;
}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	// Populate sprite batches
	PROFILE(SESSION_RENDER_ECS, pRenderer->SpriteBatchRender(m_pWorld->GetSystemByComponent<SpriteRenderComponent>()));

	// Render your sprite batches
	Profiler::GetInstance()->BeginSubSession<SESSION_BATCH_RENDERING>();
	PROFILE(SESSION_BATCH_DYNAMIC, m_pDynamic_SB->Render());
	PROFILE(SESSION_BATCH_STATIC, m_pStatic_SB->Render());
	Profiler::GetInstance()->EndSubSession();
}

void MainGame::Shutdown()
{
	m_pDynamic_SB->Destroy();
	Memory::Delete(m_pDynamic_SB);

	m_pStatic_SB->Destroy();
	Memory::Delete(m_pStatic_SB);
}
