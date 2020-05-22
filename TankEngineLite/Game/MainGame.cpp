#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

#include "Model.h"

#include "Profiler.h"
#include "BinaryInterfaces.h"

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
		WorldSystem<CameraComponent, 8, 8, ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ModelRenderComponent, 8, 9, ExecutionStyle::SYNCHRONOUS>
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

	// Create 3D camera
	{
		auto pCamera = m_pWorld->CreateEntity();
		auto [pCameraComponent, pTransform] = pCamera->PushComponents<CameraComponent, TransformComponent>();
		Renderer::GetInstance()->GetDirectX()->SetCamera(pCameraComponent);
		pTransform->Translate({ 0.f, -5.f, 0.f });
		m_pCameraTransform = pTransform;
		m_pCamera = pCameraComponent;
		m_IntendedPosition = { 0.f, -5.f, 0.f };
	}

	// Create world model
	{
		auto pModel = RESOURCES->Get<Model>("arcade");
		auto pTexture = RESOURCES->Get<Texture>("arcade_diffuse_pog");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(0.f, XM_PIDIV2, 0.f);
		pTransform->scale = { 0.01f, 0.01f, 0.01f };
	}

	// Create world model
	{
		auto pModel = RESOURCES->Get<Model>("skydome");
		auto pTexture = RESOURCES->Get<Texture>("skydome_diffuse");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(XM_PI, XM_PIDIV2, 0.f);
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

	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_C, ActionType::PRESSED,
			[this]()
			{
				if (m_Animating)
					return;

				constexpr XMFLOAT3 startPosition{ 0.f, -5.f, 0.f };
				constexpr XMFLOAT3 endPosition{ 0.f, -5.f, 22.5f };

				if (m_IsDocked)
				{
					m_IntendedPosition = endPosition;

					m_pDynamic_SB->SetIsRendering(false);
					m_pStatic_SB->SetIsRendering(false);
				}
				else
				{
					m_IntendedPosition = startPosition;
					m_Animating = true;
				}

				m_IsDocked = !m_IsDocked;
			}));
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	PROFILE(SESSION_UPDATE_ECS, ECS::Universe::GetInstance()->Update(dt));

	auto [x, y, state] = pInputManager->GetMouseState();
	m_pParticleEmitterTransform->position.x = (float)x;
	m_pParticleEmitterTransform->position.y = (float)y;

	CameraTransitions(dt);
}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	// Populate sprite batches
	PROFILE(SESSION_RENDER_ECS, pRenderer->SpriteBatchRender(m_pWorld->GetSystemByComponent<SpriteRenderComponent>()));
	PROFILE(SESSION_RENDER_ECS, pRenderer->ModelRender(m_pWorld->GetSystemByComponent<ModelRenderComponent>()));

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

void MainGame::CameraTransitions(float dt)
{
	static float timer = 0.f;

	XMStoreFloat3(&m_pCameraTransform->position, XMVectorLerp(XMLoadFloat3(&m_pCameraTransform->position), XMLoadFloat3(&m_IntendedPosition), dt));

	if (m_Animating)
		timer += dt;

	if (timer > 4.f)
	{
		// Refresh small platform
		// TODO(tomas): figure out why i need this, something in sb is breaking
		for (int i = 0; i < 25; ++i)
			m_pStatic_SB->PushSprite({ 0, 0, 16, 16 }, { (float)i * 64, 600, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });

		for (int j = 1; j < 5; ++j)
		{
			for (int i = 0; i < 25; ++i)
				m_pStatic_SB->PushSprite({ 16, 0, 32, 16 }, { (float)i * 64, 600 + (float)j * 64, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });
		}

		m_pDynamic_SB->SetIsRendering(true);
		m_pStatic_SB->SetIsRendering(true);
		timer -= timer;
		m_Animating = false;
	}
}
