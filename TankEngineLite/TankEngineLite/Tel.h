#ifndef TEL_H
#define TEL_H

#include <chrono>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"

#include "ResourceManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "SpriteBatch.h"
#include "Renderer.h"

#include "MemoryTracker.h"

#include "SoundManager.h"

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
		m_pGame = Memory::New<T>();
		new(m_pGame) T();

		// Initialized the resource manager with the root directory
		ResourceManager::GetInstance()->Init("../Resources/", L"../Resources/");

		// Root initialized and game initialize
		Initialize();

		// Load all the resources in folder
		ResourceManager::GetInstance()->LoadAllInFolder();

		// Feed this resource manager in to the game loader
		LoadGame();

		{
			auto pRenderer = Renderer::GetInstance();
			auto pInput = InputManager::GetInstance();
			auto pAudio = SoundManager::GetInstance();
			auto pProfiler = Profiler::GetInstance();
			bool done = false;

			std::chrono::duration<float> dt{};

			while (!done)
			{
				std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
				const auto currentTime = high_resolution_clock::now();
				
				// Begin profiling session
				pProfiler->BeginSession();
				
				pProfiler->BeginSubSession<SessionId::SESSION_PROCESS_INPUT>();
				done = pInput->ProcessInput();
				pProfiler->EndSubSession();

				//////////////////////////////////////////////////////////////////////////
				// ImGui update and Game Update 

				// Start the new ImGui frame
				pProfiler->BeginSubSession<SessionId::SESSION_IMGUI_NEWFRAME>();
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplSDL2_NewFrame(m_pWindow);
				ImGui::NewFrame();
				pProfiler->EndSubSession();

				// Update the game
				pProfiler->BeginSubSession<SessionId::SESSION_UPDATE>();
				
				// Input update
				pInput->Update(dt.count());
				
				// Game update
				PROFILE(SESSION_UPDATE_GAME, m_pGame->Update(dt.count(), pInput));

				// Audio update
				pAudio->Update(dt.count());
				pProfiler->EndSubSession();

				// Debug ui
				pProfiler->BeginSubSession<SessionId::SESSION_PROFILER>();
				pProfiler->Report(m_DebugProfiler);
				
				ImGuiDebug(dt.count());
				pProfiler->EndSubSession();

				//////////////////////////////////////////////////////////////////////////
				// Rendering

				// Root render for ImGui and engine related rendering
				pProfiler->BeginSubSession<SessionId::SESSION_RENDER>();
				
				pRenderer->RootRenderBegin();

				// Game related rendering
				m_pGame->Render(pRenderer);

				// End rendering cycle
				pRenderer->RootRenderEnd();

				pProfiler->EndSubSession();

				// Time compensation
				std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
				dt = t2 - t1;

				pProfiler->EndSession();
			}
		}

		Cleanup();
	}

	// Debug ui
	void ImGuiDebug(float dt);
	inline void RegisterBatch(SpriteBatch* pBatch) { m_BatchRegistry.push_back(pBatch); };

private:
	SDL_Window* m_pWindow;
	Game* m_pGame;

	// Debug ui stuff
	bool m_DebugSystems = false;
	bool m_DebugRenderer = false;
	bool m_ShowLogger = false;
	bool m_DebugMemoryTracker = false;
	bool m_DebugProfiler = false;

	std::vector<SpriteBatch*> m_BatchRegistry;
};

#endif // !TEL_H
