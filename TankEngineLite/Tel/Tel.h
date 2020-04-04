#ifndef TEL_H
#define TEL_H

#include <chrono>
using namespace std::chrono;

struct SDL_Window;
class TextComponent;
class ECS::World;

class ResourceManager;
class InputManager;
class Renderer;

// TODO(tomas): allow multiple games on one engine runner, <Game...>, implement switching

class Game
{
public:
	virtual void Initialize() {};
	virtual void Load(ResourceManager*) {};
	virtual void Update(float, InputManager*) {};
	virtual void Render(Renderer*) {};
};

class TEngineRunner
{
public:
	void Initialize();
	void LoadGame();
	void Cleanup();

	template<typename T>
	void Run()
	{
		m_pGame = new T();

		// Root initialized and game initialize
		Initialize();

		// Initialized the resource manager with the root directory
		auto pResourceManager = ResourceManager::GetInstance();
		ResourceManager::GetInstance()->Init("../Data/");

		// Feed this resource manager in to the game loader
		LoadGame();
		m_pGame->Load(pResourceManager);

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
				m_pGame->Update(dt.count(), pInput);
				m_pGame->Render(pRenderer);

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