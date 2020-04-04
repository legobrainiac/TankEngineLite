#include "TelPCH.h"
#include "MainGame.h"

void MainGame::Initialize()
{
	// Initialized world and world systems
	m_pWorld = ECS::Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		ECS::WorldSystem<TransformComponent, 2048, 0, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<RenderComponent, 2048, 1, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<LifeSpan, 2048, 2, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<TextComponent, 128, 3, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<MovementComponent, 128, 4, ECS::SystemExecutionStyle::SYNCHRONOUS>
	>();
}

void MainGame::Load(ResourceManager* pResourceManager)
{
	_(pResourceManager);

	// Background
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pRenderer, pTransform] = pEntity->PushComponents<RenderComponent, TransformComponent>();
		pRenderer->SetTexture(ResourceManager::GetInstance()->LoadTexture("Background2.png"));
	}

	// Dae logo
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pRenderer, pTransform] = pEntity->PushComponents<RenderComponent, TransformComponent>();
		pRenderer->SetTexture(ResourceManager::GetInstance()->LoadTexture("logo.png"));
		pTransform->position = { 216.f, 180.f, 0.f };
	}

	// Title
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pTextComponent, pRenderer, pTransform] = pEntity->PushComponents<TextComponent, RenderComponent, TransformComponent>();
		pTransform->position = { 80.f, 20.f, 0.f };
		pTextComponent->Initialize("Tank Engine Lite", ResourceManager::GetInstance()->LoadFont("Lingua.otf", 36));
	}

	// Fps
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pTextComponent, pRenderer, pTransform] = pEntity->PushComponents<TextComponent, RenderComponent, TransformComponent>();
		pTransform->position = { 10.f, 10.f, 0.f };
		m_pFpsTextComponent = pTextComponent;
		pTextComponent->Initialize(" big lmao ", ResourceManager::GetInstance()->LoadFont("Lingua.otf", 20), { 255, 255, 255 });
	}

	// Ross boi
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<MovementComponent, RenderComponent, TransformComponent>();
		pRenderer->SetTexture(ResourceManager::GetInstance()->LoadTexture("ROSS1.png"));
		pTransform->position = { 100.f, 100.f, 0.f };
	}

	auto pInputMananager = InputManager::GetInstance();
	pInputMananager->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_K, ActionType::PRESSED,
			[]()
			{
				LOGINFO("PRESSED K" << std::endl);
			}));

	pInputMananager->RegisterActionMappin(
		ActionMapping(SDL_SCANCODE_K, ActionType::RELEASED,
			[]()
			{
				LOGINFO("RELEASED K" << std::endl);
			}));
}

void MainGame::Update(float dt, InputManager* pInputManager)
{
	_(pInputManager);

	ECS::Universe::GetInstance()->Update(dt);

	// Fps update
	std::string text = std::to_string((int)(1 / dt)) + " FPS";
	m_pFpsTextComponent->SetText(text, { 255, 255, 0 });
}

void MainGame::Render(Renderer* pRenderer)
{
	pRenderer->Render(m_pWorld->GetSystemByComponent<RenderComponent>());
}
