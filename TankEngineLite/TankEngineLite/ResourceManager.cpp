#include "pch.h"
#include "ResourceManager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <filesystem>

#include "Renderer.h"
#include "Texture.h"
#include "Model.h"
#include "Effect.h"

std::map<std::string, std::function<void(std::string, std::string)>> ResourceManager::TypeResolvers
{
	std::pair(".fx", [](std::string path, std::string name) { RESOURCES->Load<Effect>(path, name); }),
	std::pair(".wav", [](std::string, std::string) { LOGGER->Log<LOG_WARNING>("Automatic .wav loading not implemented"); }),
	std::pair(".temd", [](std::string path, std::string name) { RESOURCES->Load<Model>(path, name); }),
	std::pair(".jpg", [](std::string path, std::string name) { RESOURCES->Load<Texture>(path, name); }),
	std::pair(".png", [](std::string path, std::string name) { RESOURCES->Load<Texture>(path, name); }),
};

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

	for (auto& sound : m_pSounds)
		sound.second->release();
}

void ResourceManager::LoadAllInFolder()
{
	for (auto& p : std::filesystem::recursive_directory_iterator("../Resources"))
	{
		std::stringstream ss;
		ss << p.path();
		LoadDecode(ss.str());
	}
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

FMOD::Sound* ResourceManager::GetSound([[maybe_unused]] const std::string& name) const
{
	const auto it = m_pSounds.find(name);

	if (it != m_pSounds.cend())
		return it->second;

	Logger::GetInstance()->Log<LOG_WARNING>("Failed to locate sound -> ", name);
	return nullptr;
}
