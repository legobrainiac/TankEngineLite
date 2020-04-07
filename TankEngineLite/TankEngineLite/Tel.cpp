#include "pch.h"
#include "Tel.h"

#include <chrono>
#include <SDL.h>

void TEngineRunner::InitializeWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());

	const auto width = 1600;
	const auto height = 900;

	m_pWindow = SDL_CreateWindow(
		"Tank Engine Mini",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		0);

	if (!m_pWindow)
		throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());

	Renderer::GetInstance()->Init(m_pWindow, width, height);
}

void TEngineRunner::Initialize()
{
	// Setup SDL with DirectX and ImGui
	InitializeWindow();

	// Initialize universe
	ECS::Universe::GetInstance();

	// Start game
	m_pGame->Initialize();
}

void TEngineRunner::LoadGame()
{
	m_pGame->Load(ResourceManager::GetInstance());
}

void TEngineRunner::Cleanup()
{
	delete m_pGame;
	ResourceManager::GetInstance()->Destroy();
	Renderer::GetInstance()->Destroy();

	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;
	SDL_Quit();
}

