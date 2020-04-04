#include "TelPCH.h"

#include <chrono>
#include <thread>
#include <SDL.h>

#include "Tel.h"
#include "Renderer.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "BasicComponents.h"

void TEngineRunner::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());

	m_pWindow = SDL_CreateWindow(
		"Tank Engine Mini",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		720,
		480,
		SDL_WINDOW_OPENGL);

	if (!m_pWindow)
		throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());

	Renderer::GetInstance()->Init(m_pWindow);
	ECS::Universe::GetInstance();
	m_pGame->Initialize();
}

void TEngineRunner::LoadGame()
{
	m_pGame->Load(ResourceManager::GetInstance());
}

void TEngineRunner::Cleanup()
{
	delete m_pGame;
	Renderer::GetInstance()->Destroy();

	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;
	SDL_Quit();
}

