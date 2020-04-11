#ifndef TEL_H
#define TEL_H

#include <chrono>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"

#include "ResourceManager.h"
#include "InputManager.h"
#include "SpriteBatch.h"
#include "Renderer.h"


using namespace std::chrono;

// TODO(tomas): allow multiple games on one engine runner, <Game...>, implement switching
// TODO(tomas): fixed update adder, create lambdas that run on a fixed update of x timestep

class TEngineRunner;

class Game
{
public:
	virtual void Initialize() {};
	virtual void Load(ResourceManager*, TEngineRunner*) {};
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
				pInput->Update(dt.count());
				m_pGame->Update(dt.count(), pInput);
				
				ImGuiDebug(dt.count());

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

	// Debug ui
	void ImGuiDebug(float dt);
	inline void RegisterBatchForDebug(SpriteBatch* pBatch) { m_BatchRegistry.push_back(pBatch); };
    
private:
	SDL_Window* m_pWindow;
	Game* m_pGame;

	// Debug ui stuff
	bool m_DebugSystems = true;
	bool m_DebugRenderer = true;
	bool m_ShowLogger = true;

	std::vector<SpriteBatch*> m_BatchRegistry;
};

#endif // !TEL_H