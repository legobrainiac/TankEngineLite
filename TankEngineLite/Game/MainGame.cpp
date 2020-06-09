#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
#include "Logger.h"
#include "imgui.h"

#include "Model.h"

#include "Profiler.h"
#include "BinaryInterfaces.h"

#include "BBLevel.h"

#define TINY	8
#define SMALL	256
#define BIG		4096

void MainGame::Initialize()
{
	using namespace ECS;

	// Initialized world and world systems
	m_pWorld = Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		WorldSystem<TransformComponent2D,	SMALL,	0,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<SpriteRenderComponent,	SMALL,	1,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<LifeSpan,				SMALL,	2,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ProjectileComponent,	SMALL,	3,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<PlayerController,		TINY,	4,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ParticleEmitter,		TINY,	5,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<Particle,				SMALL,	6,	ExecutionStyle::ASYNCHRONOUS>,
		WorldSystem<TransformComponent,		TINY,	7,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<CameraComponent,		TINY,	8,	ExecutionStyle::SYNCHRONOUS>,
		WorldSystem<ModelRenderComponent,	TINY,	9,	ExecutionStyle::SYNCHRONOUS>
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

	//////////////////////////////////////////////////////////////////////////
	// Load level
	m_pCurrentLevel = RESOURCES->Get<BBLevel>("wiki");
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
		m_IntendedPosition = { 0.f, -5.f, 0.f };
	}

	//////////////////////////////////////////////////////////////////////////
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

	//////////////////////////////////////////////////////////////////////////
	// Create sky dome model
	{
		auto pModel = RESOURCES->Get<Model>("skydome");
		auto pTexture = RESOURCES->Get<Texture>("skydome_diffuse");

		auto pWorldModel = m_pWorld->CreateEntity();
		auto [pModelRenderer, pTransform] = pWorldModel->PushComponents<ModelRenderComponent, TransformComponent>();
		pModelRenderer->Initialize(pModel, pTexture);
		pTransform->Rotate(XM_PI, XM_PIDIV2, 0.f);
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
				m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { float(rand() % 1000 + 300), 0 }, (Player)controller, m_pCurrentLevel);
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
		m_pCurrentLevel->SetupBatch(m_pStatic_SB); // TODO(tomas): replace batch session, shouldnt have to do this
		m_pDynamic_SB->SetIsRendering(true);
		m_pStatic_SB->SetIsRendering(true);
		timer -= timer;
		m_Animating = false;
	}
}
