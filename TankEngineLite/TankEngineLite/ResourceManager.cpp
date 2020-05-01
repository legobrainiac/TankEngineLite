#include "pch.h"
#include "ResourceManager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "Renderer.h"
#include "Texture.h"
void ResourceManager::Init(const std::string& dataPath, const std::wstring& dataW)
{
	LOGGER->Log<LOG_INFO>("Initializing resource manager");

	m_DataPath = dataPath;
	m_DataPathW = dataW;

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
	for (auto& r : m_Resources)
		r.second.Cleanup();

	for (auto& effect : m_pEffects)
		DXRELEASE(effect.second);

	for (auto& sound : m_pSounds)
		sound.second->release();
}

//////////////////////////////////////////////////////////////////////////
// Effect Loading
ID3DX11Effect* ResourceManager::LoadEffect(const std::wstring& file, const std::string& name)
{
	HRESULT hr = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Convert to wchar_t*
	std::wstring fullPath = m_DataPathW + file;
	
	hr = D3DX11CompileEffectFromFile(
		fullPath.c_str(),
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
			DXRELEASE(pErrorBlob);

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wcout << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << fullPath << std::endl;
			return nullptr;
		}
	}
	
	LOGGER->Log<LOG_SUCCESS>("Loaded " + name);

	DXRELEASE(pErrorBlob);
	m_pEffects[name] = pEffect;
	
	return pEffect;
}

ID3DX11Effect* ResourceManager::GetEffect(const std::string& name) const
{
	const auto it = m_pEffects.find(name);

	if (it != m_pEffects.cend())
		return it->second;

	Logger::GetInstance()->Log<LOG_WARNING>("Failed to locate effect -> ", name);
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Sound loading
FMOD::Sound* ResourceManager::LoadSound(const std::string& file, const std::string& name)
{
	// If texture already exists, we don't do anything, just return it
	const auto it = m_pSounds.find(name);

	if (it != m_pSounds.cend())
		return it->second;

	// Otherwise
	const auto fullPath = m_DataPath + file;
	const auto pSoundSystem = SoundManager::GetInstance()->GetSystem();

	FMOD::Sound* pSound = nullptr;
	auto result = pSoundSystem->createSound(fullPath.c_str(), FMOD_DEFAULT, nullptr, &pSound);
	
	// Create sound
	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("Failed to load sound ", name);
		return nullptr;
	}
	else
		LOGGER->Log<LOG_SUCCESS>("Loaded sound ", name);

	m_pSounds[name] = pSound;
	return pSound;
}

FMOD::Sound* ResourceManager::GetSound([[maybe_unused]]const std::string& name) const
{
	const auto it = m_pSounds.find(name);

	if (it != m_pSounds.cend())
		return it->second;

	Logger::GetInstance()->Log<LOG_WARNING>("Failed to locate sound -> ", name);
	return nullptr;
}