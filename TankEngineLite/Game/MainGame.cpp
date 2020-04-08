#include "pch.h"
#include "MainGame.h"

#include "SpriteBatch.h"
#include "Prefabs.h"
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
		ECS::WorldSystem<PlayerController, 8, 4, ECS::SystemExecutionStyle::SYNCHRONOUS>
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
	for (int j = 0; j < 5; ++j)
	{
		for (int i = 0; i < 25; ++i)
			m_pBackgroundStatic_SB->PushSprite({ 0, 0, 16, 16 }, { (float)i * 64, 600 + (float)j * 64, 0.9f }, 0, { 4, 4 }, { 0, 0 }, { 1.f, 1.f, 1.f, 1.f });
	}

	// TODO(tomas): player connected callback instead of this
	// Spawn player one
	Prefabs::CreatePlayer(m_pWorld, m_pCharacter_SB, { 1600 / 2 - 32, 0 }, Player::PLAYER0);
	Prefabs::CreatePlayer(m_pWorld, m_pCharacter_SB, { 1600 / 2 + 32, 0 }, Player::PLAYER1);
	Prefabs::CreatePlayer(m_pWorld, m_pCharacter_SB, { 1600 / 2 - 96, 0 }, Player::PLAYER2);
	Prefabs::CreatePlayer(m_pWorld, m_pCharacter_SB, { 1600 / 2 + 96, 0 }, Player::PLAYER3);
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
	// Populate sprite batches
	pRenderer->SpriteBatchRender(m_pWorld->GetSystemByComponent<SpriteRenderComponent>());

	// Render your sprite batches
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
