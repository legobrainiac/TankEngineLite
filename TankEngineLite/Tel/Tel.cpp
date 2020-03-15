#include "TelPCH.h"

#include <chrono>
#include <thread>
#include <SDL.h>

#include "Tel.h"
#include "Renderer.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "BasicComponents.h"

using namespace std::chrono;

void TEngine::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());

	m_pWindow = SDL_CreateWindow(
		"Tank Engine Mini",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_OPENGL);

	if (!m_pWindow)
		throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());

	Renderer::GetInstance()->Init(m_pWindow);
}

/**
 * Code constructing the scene world starts here
 */
void TEngine::LoadGame()
{
	m_pWorld = new ECS::WorldSystem();

	// Background
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pRenderer, pTransform] = pEntity->PushComponents<RenderComponent, TransformComponent>();
		pRenderer->SetTexture(ResourceManager::GetInstance()->LoadTexture("background.jpg"));
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
		pTextComponent->Initialize("Programming 4 assignment", ResourceManager::GetInstance()->LoadFont("Lingua.otf", 36));
	}

	// Fps
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pTextComponent, pRenderer, pTransform] = pEntity->PushComponents<TextComponent, RenderComponent, TransformComponent>();
		pTransform->position = { 10.f, 10.f, 0.f };
		m_pFpsTextComponent = pTextComponent;
		pTextComponent->Initialize(" big lmao ", ResourceManager::GetInstance()->LoadFont("Lingua.otf", 20), { 255, 255, 0 });
	}
    
    // Dae logo
	{
		auto pEntity = m_pWorld->CreateEntity();
		auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<MovementComponent, RenderComponent, TransformComponent>();
		pRenderer->SetTexture(ResourceManager::GetInstance()->LoadTexture("ROSS1.png"));
		pTransform->position = { 100.f, 100.f, 0.f };
	}
    
}

void TEngine::Cleanup()
{
	Renderer::GetInstance()->Destroy();

	// Delete singletons i guess
	delete Renderer::GetInstance();
	delete InputManager::GetInstance();
	delete ResourceManager::GetInstance();

	delete m_pWorld;

	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;
	SDL_Quit();
}

void TEngine::Run()
{
	Initialize();

	// Tell the resource manager where he can find the game data
	ResourceManager::GetInstance()->Init("../Data/");

	LoadGame();

	{
		auto renderer = Renderer::GetInstance();
		auto input = InputManager::GetInstance();

		bool done = false;

		std::chrono::duration<float> dt{};
		while (!done)
		{
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

			const auto currentTime = high_resolution_clock::now();

			done = input->ProcessInput();
			m_pWorld->Update(dt.count());
			renderer->Render(m_pWorld);

			// Fps update
			std::string text = std::to_string((int)(1 / dt.count())) + " FPS";
			m_pFpsTextComponent->SetText(text, { 255, 255, 0 });

			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			dt = t2 - t1;
		}
	}

	Cleanup();
}
