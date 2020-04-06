#include "pch.h"
#include "MainGame.h"

#include "imgui.h"
#include "SpriteBatch.h"

void MainGame::Initialize()
{
	// Initialized world and world systems
	m_pWorld = ECS::Universe::GetInstance()->PushWorld();

	m_pWorld->PushSystems<
		ECS::WorldSystem<TransformComponent, 2048, 0, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<RenderComponent, 2048, 1, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<LifeSpan, 2048, 2, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<TextComponent, 128, 3, ECS::SystemExecutionStyle::SYNCHRONOUS>,
		ECS::WorldSystem<MovementComponent, 8, 4, ECS::SystemExecutionStyle::SYNCHRONOUS>
	>();
}

void MainGame::Load([[maybe_unused]] ResourceManager* pResourceManager)
{
	// Load Bubble Bobble texture atlases
 	m_pSpriteBatch = new SpriteBatch();	
 	m_pSpriteBatch->InitializeBatch(ResourceManager::GetInstance()->LoadTexture("atlas_0.png", "atlas_0"));

	// Ross boi
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<MovementComponent, RenderComponent, TransformComponent>();
		pRenderer->SetSpriteBatch(m_pSpriteBatch);
		pRenderer->SetAtlasTransform({0, 0, 16, 16});
		pTransform->position = { 0.f, 0.f, 0.f };
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

void MainGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	ECS::Universe::GetInstance()->Update(dt);


}

void MainGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	pRenderer->Render(m_pWorld->GetSystemByComponent<RenderComponent>());
	m_pSpriteBatch->Render();
}

void MainGame::Shutdown()
{
	m_pSpriteBatch->Destroy();
	delete m_pSpriteBatch;
}
