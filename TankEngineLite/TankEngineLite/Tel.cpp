#include "pch.h"
#include "Tel.h"

#include <chrono>
#include <SDL.h>

#include "Texture.h"

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

	Renderer::GetInstance()->Init(m_pWindow, width, height, false);
}

void TEngineRunner::Initialize()
{
	// Setup SDL with DirectX and ImGui
	InitializeWindow();
	SoundManager::GetInstance()->Init();

	// Initialize universe
	ECS::Universe::GetInstance();

	// Start game
	m_pGame->Initialize();
}

void TEngineRunner::LoadGame()
{
	m_pGame->Load(ResourceManager::GetInstance(), this);
}

void TEngineRunner::Cleanup()
{
	SDL_DestroyWindow(m_pWindow);
	SDL_Quit();
	
	m_pGame->Shutdown();
	Profiler::GetInstance()->Destroy();
	ResourceManager::GetInstance()->Destroy();
	SoundManager::GetInstance()->Destroy();
	Renderer::GetInstance()->Destroy();

	Memory::Delete(m_pGame);
}

void TEngineRunner::ImGuiDebug(float dt)
{
	//////////////////////////////////////////////////////////////////////////
	// ImGui debug rendering

	if (m_ShowLogger)
		Logger::GetInstance()->UpdateAndDraw();

	auto [x, y, s] = InputManager::GetInstance()->GetMouseState();

	if (y < 120)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Files"))
			{
				if (ImGui::MenuItem("Load script", "CTRL+O"))
					Logger::GetInstance()->Log<LOG_WARNING>("Script loading not implemented");

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Toggle All", "")) 
				{ 
					m_DebugSystems = !m_DebugSystems; 
					m_DebugRenderer = !m_DebugRenderer;
					m_ShowLogger = !m_ShowLogger;
					m_DebugProfiler = !m_DebugProfiler;
					m_DebugMemoryTracker = !m_DebugMemoryTracker;
				}

				if (ImGui::MenuItem("System debugger", "")) { m_DebugSystems = !m_DebugSystems; }
				if (ImGui::MenuItem("Renderer settings", "")) { m_DebugRenderer = !m_DebugRenderer; }  // Disabled item
				if (ImGui::MenuItem("Logger", "")) { m_ShowLogger = !m_ShowLogger; }  // Disabled item
				if (ImGui::MenuItem("Profiler", "")) { m_DebugProfiler = !m_DebugProfiler; }  // Disabled item
				if (ImGui::MenuItem("Memory tracker", "")) { m_DebugMemoryTracker = !m_DebugMemoryTracker; }  // Disabled item

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	if (m_DebugSystems)
		ECS::Universe::GetInstance()->ImGuiDebug();

	if (m_DebugMemoryTracker)
	{
		ImGui::Begin("Memory tracker");
		const auto status = Memory::GetMemoryStatus();
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Memory::m_TotalMemoryCount = ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(status.m_TotalMemoryCount * 0.000001f).c_str());
		ImGui::SameLine();
		ImGui::Text("MB");

		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Memory::m_LiveMemoryChuncks = ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(status.m_MemoryChunckCount).c_str());
		ImGui::End();
	}

	if (m_DebugRenderer)
	{
		ImGui::Begin("Renderer settings");

		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "DT: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(dt).c_str());
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "FPS: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(ImGui::GetIO().Framerate).c_str());

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Sprite batches");
		if (ImGui::BeginTabBar("Sprite batches", ImGuiTabBarFlags_None))
		{
			for (auto pBatch : m_BatchRegistry)
			{
				if (ImGui::BeginTabItem(pBatch->GetName().data()))
				{
					pBatch->ImGuiDebug();
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

