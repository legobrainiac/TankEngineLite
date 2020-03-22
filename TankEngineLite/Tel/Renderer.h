#ifndef RENDER_H
#define RENDER_H

#include "Singleton.h"
#include "ecs.h"

struct SDL_Window;
struct SDL_Renderer;

class Texture2D;

/**
 * Simple RAII wrapper for the SDL renderer
 */
class Renderer : public Singleton<Renderer>
{
public:
	void Init(SDL_Window* window);
	void Render() const;
	void Render(ECS::System* pWorldRenderSystem) const;
	void Destroy();

	void RenderTexture(Texture2D* pTexture, float x, float y) const;
	void RenderTexture(Texture2D* pTexture, float x, float y, float width, float height) const;

	SDL_Renderer* GetSDLRenderer() const { return m_Renderer; }
private:
	SDL_Renderer* m_Renderer{};
};

#endif // !RENDER_H