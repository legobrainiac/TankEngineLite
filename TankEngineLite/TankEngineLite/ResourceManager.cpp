#include "pch.h"
#include "ResourceManager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Renderer.h"
#include "Texture.h"
#include "Font.h"

void ResourceManager::Init(const std::string& dataPath)
{
	m_DataPath = dataPath;

	// Load support for png and jpg, this takes a while!
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) 
		throw std::runtime_error(std::string("Failed to load support for png's: ") + SDL_GetError());

	if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) != IMG_INIT_JPG) 
		throw std::runtime_error(std::string("Failed to load support for jpg's: ") + SDL_GetError());

	if (TTF_Init() != 0) 
		throw std::runtime_error(std::string("Failed to load support for fonts: ") + SDL_GetError());
}

void ResourceManager::Destroy()
{
	for (auto& tex : m_pTextures)
	{
		tex.second->Shutdown();
		delete tex.second;
	}

	for (auto& effect : m_pEffects)
		DXRELEASE(effect.second);
}

//////////////////////////////////////////////////////////////////////////
// Texture loading
Texture* ResourceManager::LoadTexture(const std::string& file, const std::string& name)
{
	// If texture already exists, we don't do anything, just return it
	const auto it = m_pTextures.find(name);

	if (it != m_pTextures.cend())
		return it->second;

	// Otherwise
	const auto fullPath = m_DataPath + file;
	auto pTexture = new Texture();
    
	try
	{
		bool result = pTexture->Initialize(Renderer::GetInstance()->GetDirectX()->GetDevice(), fullPath.c_str());

		if (pTexture == nullptr || !result)
			throw std::runtime_error(std::string("Failed to load texture: " + file + " as " + name));

		m_pTextures[name] = pTexture;
	}
	catch (const std::exception& e)
	{
		Logger::GetInstance()->Log<LOG_ERROR>(e.what());

		// Fall back exception, 
		//  we don't want memory leaks on failed texture creation
		delete pTexture;

		return nullptr;
	}

	return pTexture;
}

inline Texture* ResourceManager::GetTexture(const std::string& name) const
{
	const auto it = m_pTextures.find(name);

	if (it != m_pTextures.cend())
		return it->second;

	Logger::GetInstance()->Log<LOG_ERROR>("Failed to locate texture -> " + name);

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Effect Loading
ID3DX11Effect* ResourceManager::LoadEffect(const std::string& file, const std::string& name)
{
	HRESULT hr;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Convert to wchar_t*
	const auto fullPath = m_DataPath + file;
	std::wstring path(fullPath.begin(), fullPath.end());
	
	hr = D3DX11CompileEffectFromFile(path.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		Renderer::GetInstance()->GetDirectX()->GetDevice(),
		&pEffect,
		&pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
		{
			auto* errors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << errors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wcout << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << path << std::endl;
			return nullptr;
		}
	}
	
	DXRELEASE(pErrorBlob)
	m_pEffects[name] = pEffect;
	
	return pEffect;
}

inline ID3DX11Effect* ResourceManager::GetEffect(const std::string& name) const
{
	const auto it = m_pEffects.find(name);

	if (it != m_pEffects.cend())
		return it->second;

	Logger::GetInstance()->Log<LOG_ERROR>("Failed to locate effect -> " + name);
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Font Loading
Font* ResourceManager::LoadFont(const std::string& file, unsigned int size) const
{
	return new Font(m_DataPath + file, size);
}
