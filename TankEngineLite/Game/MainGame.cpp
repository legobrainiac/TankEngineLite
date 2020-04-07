#include "pch.h"
#include "MainGame.h"

#include "imgui.h"
#include "SpriteBatch.h"

void MainGame::Initialize()
{
	// Initialized world and world systems
	m_pWorld = ECS::Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		ECS::WorldSystem<TransformComponent2D, 512, 0, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<RenderComponent, 512, 1, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<LifeSpan, 512, 2, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<MovementComponent, 8, 4, ECS::SystemExecutionStyle::SYNCHRONOUS>
	>();
}

void MainGame::Load([[maybe_unused]] ResourceManager* pResourceManager)
{
	// Create the sprite batches
	m_pCharacter_SB = new SpriteBatch();
	m_pCharacter_SB->InitializeBatch(ResourceManager::GetInstance()->LoadTexture("atlas_0.png", "atlas_0"));

	m_pBackgroundStatic_SB = new SpriteBatch();
	m_pBackgroundStatic_SB->InitializeBatch(ResourceManager::GetInstance()->LoadTexture("atlas_5.png", "atlas_5"), BatchMode::BATCHMODE_STATIC);

	// Setup a smol platform
	for (int i = 0; i < 25; ++i)
		m_pBackgroundStatic_SB->PushSprite({ 0, 0, 16, 16 }, { (float)i * 64, 600, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });
	
	// Ross boi
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<MovementComponent, RenderComponent, TransformComponent2D>();
		pRenderer->SetSpriteBatch(m_pCharacter_SB);
		pRenderer->SetAtlasTransform({ 0, 0, 16, 16 });
		pTransform->position = { 1600.f / 2.f, 900.f / 2.f, 0.f };
		pTransform->scale = { 4.f, 4.f };
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
			[&]()
			{
				LOGINFO("RELEASED K" << std::endl);
				auto pEntity = m_pWorld->CreateEntity();
				auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<MovementComponent, RenderComponent, TransformComponent2D>();
				pRenderer->SetSpriteBatch(m_pCharacter_SB);
				pRenderer->SetAtlasTransform({ 0, 0, 16, 16 });
				pTransform->position = { 1600.f / 2.f, 900.f / 2.f, 0.f };
			}));
}

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	ECS::Universe::GetInstance()->Update(dt);

	ImGui::Begin("Render stats");

	std::stringstream stream;
	stream << "DT: " << dt;
	ImGui::Text(stream.str().c_str());

	ImGui::End();
}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	pRenderer->Render(m_pWorld->GetSystemByComponent<RenderComponent>());
	m_pCharacter_SB->Render();
	m_pBackgroundStatic_SB->Render();
}

void MainGame::Shutdown()
{
	m_pCharacter_SB->Destroy();
	delete m_pCharacter_SB;

	m_pBackgroundStatic_SB->Destroy();
	delete m_pCharacter_SB;
}
