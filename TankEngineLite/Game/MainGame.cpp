#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

void MainGame::Initialize()
{
	// Initialized world and world systems
	m_pWorld = ECS::Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		ECS::WorldSystem<TransformComponent2D, 256, 0, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<SpriteRenderComponent, 256, 1, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<LifeSpan, 256, 2, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<ProjectileComponent, 256, 3, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<PlayerController, 8, 4, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<ParticleEmitter, 16, 5, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<Particle, 2048, 6, ECS::SystemExecutionStyle::ASYNCHRONOUS/*TODO(tomas): async execution*/>
	>();
}

void MainGame::Load([[maybe_unused]] ResourceManager* pResourceManager, [[maybe_unused]] TEngineRunner* pEngine)
{
	// Create the sprite batches
	m_pDynamic_SB = new SpriteBatch("Dynamic");
	auto pTexDyn = ResourceManager::GetInstance()->GetTexture("atlas_0");
	if (!pTexDyn)
		pTexDyn = ResourceManager::GetInstance()->LoadTexture("atlas_0.png", "atlas_0");
	m_pDynamic_SB->InitializeBatch(pTexDyn);
	pEngine->RegisterBatchForDebug(m_pDynamic_SB);

	m_pStatic_SB = new SpriteBatch("Static");
	auto pTexStat = ResourceManager::GetInstance()->GetTexture("atlas_5");
	if(!pTexStat)
		pTexStat = ResourceManager::GetInstance()->LoadTexture("atlas_5.png", "atlas_5");
	m_pStatic_SB->InitializeBatch(pTexStat, BatchMode::BATCHMODE_STATIC);
	pEngine->RegisterBatchForDebug(m_pStatic_SB);

	// Create particle system
	{
		auto pParticleSystemEntity = m_pWorld->CreateEntity();
		auto[pParticleEmitter, pTransform] = pParticleSystemEntity->PushComponents<ParticleEmitter, TransformComponent2D>();
		pParticleEmitter->m_ParticleSpawnInterval = 0.01f;
		pParticleEmitter->m_ParticleLifeTime = .5f;
		pParticleEmitter->m_ParticlesPerSpawn = 5U;
		pParticleEmitter->m_pSpriteBatch = m_pDynamic_SB;
		pParticleEmitter->m_Gravity = 981.f;

		m_pParticleEmitterTransform = pTransform;
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
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	ECS::Universe::GetInstance()->Update(dt);

	auto [x, y, state] = pInputManager->GetMouseState();
	m_pParticleEmitterTransform->position.x = (float)x;
	m_pParticleEmitterTransform->position.y = (float)y;
}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	// Populate sprite batches
	pRenderer->SpriteBatchRender(m_pWorld->GetSystemByComponent<SpriteRenderComponent>());

	// Render your sprite batches
	m_pDynamic_SB->Render();
	m_pStatic_SB->Render();
}

void MainGame::Shutdown()
{
	m_pDynamic_SB->Destroy();
	delete m_pDynamic_SB;

	m_pStatic_SB->Destroy();
	delete m_pStatic_SB;
}
