#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

#include "Model.h"

#include "Profiler.h"
#include "BinaryInterfaces.h"

#include "ColliderComponent.h"
#include "PlayerController.h"

#include "BBLevel.h"

#define TINY	8
#define SMALL	256
#define MED		1024
#define BIG		4096

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
		WorldSystem<ModelRenderComponent, TINY, 10, ExecutionStyle::SYNCHRONOUS>
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
	m_pCurrentLevel = RESOURCES->Get<BBLevel>("tech");
	m_pCurrentLevel->SetupBatch(m_pStatic_SB);

	//////////////////////////////////////////////////////////////////////////
	// Create 3D camera
	{
		auto pCamera = m_pWorld->CreateEntity();
		auto [pCameraComponent, pTransform] = pCamera->PushComponents<CameraComponent, TransformComponent>();
		Renderer::GetInstance()->GetDirectX()->SetCamera(pCameraComponent);
		pTransform->Translate({ 0.f, -5.f, 0.f });
		m_pCameraTransform = pTransform;
		m_pCamera = pCameraComponent;
		m_IntendedPosition = { 0.f, -5.f, 20.f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Create world model
	{
		[[maybe_unused]] auto pModel = RESOURCES->Get<Model>("arcade");
		[[maybe_unused]] auto pTexture = RESOURCES->Get<Texture>("arcade_diffuse_pog");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(0.f, XM_PIDIV2, 0.f);
		pTransform->scale = { 0.01f, 0.01f, 0.01f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Create sky dome model
	{
		auto pModel = RESOURCES->Get<Model>("skydome");
		auto pTexture = RESOURCES->Get<Texture>("skydome_diffuse");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(0.f, XM_PIDIV2, 0.f);
		pTransform->scale = { 0.05f, 0.05f, 0.05f };
	}

	//////////////////////////////////////////////////////////////////////////
	// If a controller is connected, 
	//  spawn a player for it
	// If a controller is disconnected
	//  destroy the corresponding player
	InputManager::GetInstance()->RegisterControllerConnectedCallback(
		[this](uint32_t controller, ConnectionType connection)
		{
			if (connection == ConnectionType::CONNECTED)
				m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { float(rand() % 1000 + 300), 90 }, (Player)controller, m_pCurrentLevel);
			else
				m_pWorld->DestroyEntity(m_pPlayers[controller]->GetId());
		});

	InputManager::GetInstance()->CheckControllerConnection();

	//////////////////////////////////////////////////////////////////////////
	// Check for new controllers
	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_T, ActionType::PRESSED,
			[]()
			{
				InputManager::GetInstance()->CheckControllerConnection();
			}));

	InputManager::GetInstance()->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_RETURN, ActionType::PRESSED,
			[this]()
			{
				m_Playing = true;
			}));
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	PROFILE(SESSION_UPDATE_ECS, ECS::Universe::GetInstance()->Update(dt));
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

	if (m_Playing)
	{
		timer += dt;
		m_IntendedPosition = { 0.f, -5.f, 0.f };

		if (timer > 3.f)
			m_IntendedScale = 1.f;
	}

	float cameraInterpDT = dt;
	XMStoreFloat3(&m_pCameraTransform->position, XMVectorLerp(XMLoadFloat3(&m_pCameraTransform->position), XMLoadFloat3(&m_IntendedPosition), cameraInterpDT));
	m_CurrentScale = Utils::Lerp(m_CurrentScale, m_IntendedScale, dt * 4);
	m_pDynamic_SB->SetScale(m_CurrentScale);
	m_pStatic_SB->SetScale(m_CurrentScale);
}
