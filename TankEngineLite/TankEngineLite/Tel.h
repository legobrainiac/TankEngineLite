#ifndef TEL_H
#define TEL_H

#include <chrono>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"

using namespace std::chrono;

struct SDL_Window;
class TextComponent;
class ECS::World;

class ResourceManager;
class InputManager;
class Renderer;

// TODO(tomas): allow multiple games on one engine runner, <Game...>, implement switching
// TODO(tomas): fixed update adder, create lambdas that run on a fixed update of x timestep
// TODO(tomas): something like this:
//     pRenderer->PushSprite(SpriteAtlas::ATLAS_0[SPRITE_BUBBLE]/*contains all the information on uv for the selected sprite*/, /*pos*/{ 0.f, 0.f, 0.f /*depth*/ }, /*scale*/{ 1.f, 1.f }, /*pivot*/{ 0.5f, 0.5f }, 45.f);
//     one call to the GPU performs the drawing on a vertex buffer with a geometry shader to generate vertices

class Game
{
public:
	virtual void Initialize() {};
	virtual void Load(ResourceManager*) {};
	virtual void Update(float, InputManager*) {};
	virtual void Render(Renderer*) {};
	virtual void Shutdown() {};
};

class TEngineRunner
{
public:
	void InitializeWindow();
	void Initialize();
	void LoadGame();
	void Cleanup();

	template<typename T>
	void Run()
	{
		m_pGame = new T();

		// Initialized the resource manager with the root directory
		ResourceManager::GetInstance()->Init("../Resources/");

		// Root initialized and game initialize
		Initialize();

		// Feed this resource manager in to the game loader
		LoadGame();

		{
			auto pRenderer = Renderer::GetInstance();
			auto pInput = InputManager::GetInstance();

			bool done = false;

			std::chrono::duration<float> dt{};
			while (!done)
			{
				std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
				const auto currentTime = high_resolution_clock::now();
				done = pInput->ProcessInput();

				//////////////////////////////////////////////////////////////////////////
				// ImGui update and Game Update 
				
				// Start the new ImGui frame
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplSDL2_NewFrame(m_pWindow);
				ImGui::NewFrame();

				// Update the game
				m_pGame->Update(dt.count(), pInput);
				
				//////////////////////////////////////////////////////////////////////////
				// Rendering
				
				// Root render for ImGui and engine related rendering
				pRenderer->RootRenderBegin();
				
				// Game related rendering
				m_pGame->Render(pRenderer);

				// End rendering cycle
				pRenderer->RootRenderEnd();

				// Time compensation
				std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
				dt = t2 - t1;
			}
		}

		Cleanup();
	}

    
private:
	SDL_Window* m_pWindow;
	Game* m_pGame;
};

#endif // !TEL_H