#ifndef TEL_H
#define TEL_H

struct SDL_Window;
class TextComponent;
class ECS::World;

class TEngine
{
public:
	void Initialize();
	void LoadGame();
	void Cleanup();
	void Run();
    
private:
	static const int MsPerFrame = 16; // 16 for 60 fps, 33 for 30 fps
	SDL_Window* m_pWindow{};
	TextComponent* m_pFpsTextComponent;
    ECS::World* m_pWorld;
};

#endif // !TEL_H