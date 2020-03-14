#include "TelPCH.h"
#include <SDL.h>
#include <SDL_ttf.h>

#include "TextObject.h"
#include "Renderer.h"
#include "Font.h"
#include "Texture2D.h"

TextObject::TextObject(const std::string& text, Font* font) 
	: m_NeedsUpdate(true), m_Text(text), m_Font(font), m_Texture(nullptr)
{ }

void TextObject::Update()
{
	if (m_NeedsUpdate)
	{
		const SDL_Color color = { 255,255,255 }; // only white text is supported now
		const auto surf = TTF_RenderText_Blended(m_Font->GetFont(), m_Text.c_str(), color);
		if (surf == nullptr) 
			throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());
        
		auto texture = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), surf);
		if (texture == nullptr) 
			throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());
        
		SDL_FreeSurface(surf);
		m_Texture = new Texture2D(texture);
		m_NeedsUpdate = false;
	}
}

void TextObject::Render() const
{
	if (m_Texture != nullptr)
	{
		const auto pos = m_Transform.GetPosition();
		Renderer::GetInstance()->RenderTexture(m_Texture, pos.x, pos.y);
	}
}

// This implementation uses the "dirty flag" pattern
void TextObject::SetText(const std::string& text)
{
	m_Text = text;
	m_NeedsUpdate = true;
}

void TextObject::SetPosition(const float x, const float y)
{
	m_Transform.SetPosition(x, y, 0.0f);
}

TextObject::~TextObject()
{
    delete m_Texture;
    delete m_Font;
}
    