#include "pch.h"
#include "Renderer.h"
#include <SDL.h>
//#include "SceneManager.h"
#include "Texture2D.h"

#include "BasicComponents.h"

void Renderer::Init(SDL_Window* window)
{
	m_Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (m_Renderer == nullptr)
		throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
}

void Renderer::Render(ECS::System* pWorldRenderSystem) const
{
	SDL_RenderClear(m_Renderer);

	if (pWorldRenderSystem)
	{
		// Render world
		pWorldRenderSystem->ForAll([](ECS::EntityComponent* pC)
			{
				static_cast<RenderComponent*>(pC)->Render();
			});
	}

	SDL_RenderPresent(m_Renderer);
}

void Renderer::Destroy()
{
	if (m_Renderer != nullptr)
	{
		SDL_DestroyRenderer(m_Renderer);
		m_Renderer = nullptr;
	}
}

void Renderer::RenderTexture(Texture2D* pTexture, const float x, const float y) const
{
	SDL_Rect dst;
	dst.x = static_cast<int>(x);
	dst.y = static_cast<int>(y);
	SDL_QueryTexture(pTexture->GetSDLTexture(), nullptr, nullptr, &dst.w, &dst.h);
	SDL_RenderCopy(GetSDLRenderer(), pTexture->GetSDLTexture(), nullptr, &dst);
}

void Renderer::RenderTexture(Texture2D* pTexture, const float x, const float y, const float width, const float height) const
{
	SDL_Rect dst;
	dst.x = static_cast<int>(x);
	dst.y = static_cast<int>(y);
	dst.w = static_cast<int>(width);
	dst.h = static_cast<int>(height);
	SDL_RenderCopy(GetSDLRenderer(), pTexture->GetSDLTexture(), nullptr, &dst);
}
