#ifndef RENDER_H
#define RENDER_H

#include "Singleton.h"
#include "ecs.h"

#include <tuple>

struct SDL_Window;
struct SDL_Renderer;

class Texture;
class D3D;

class Renderer
	: public Singleton<Renderer>
{
public:
	bool Init(SDL_Window* window, int xw, int yh);
	void Render(ECS::System* pWorldRenderSystem) const;
	void RootRenderBegin() const;
	void RootRenderEnd() const;
	void Destroy();

	void RenderTexture(Texture* pTexture, float x, float y) const;
	void RenderTexture(Texture* pTexture, float x, float y, float width, float height) const;

	inline D3D* GetDirectX() const { return m_pDirectX; }
	inline std::tuple<int, int> GetScreenSize() { return std::make_tuple(m_Xw, m_Yh); }

private:
	SDL_Window* m_pWindow;
	D3D* m_pDirectX;

	int m_Xw, m_Yh;
};

#endif // !RENDER_H