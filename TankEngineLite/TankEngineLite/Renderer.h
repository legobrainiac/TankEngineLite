#ifndef RENDER_H
#define RENDER_H

#include <tuple>

#include "Singleton.h"
#include "ecs.h"

struct SDL_Renderer;
struct SDL_Window;

class Texture;
class D3D;

class Renderer
	: public Singleton<Renderer>
{
public:
	bool Init(SDL_Window* window, int xw, int yh, bool vSync = false);
	void SpriteBatchRender(ECS::System* pWorldRenderSystem) const;
	void RootRenderBegin() const;
	void RootRenderEnd() const;
	void Destroy();

	inline D3D* GetDirectX() const { return m_pDirectX; }
	inline std::tuple<int, int> GetScreenSize() { return std::make_tuple(m_Xw, m_Yh); }

private:

	SDL_Window* m_pWindow;
	D3D* m_pDirectX;

	int m_Xw, m_Yh;
};

#endif // !RENDER_H