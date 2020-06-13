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
#include "Sound.h"

std::map<std::string_view, TypeResolver> ResourceManager::TypeResolvers
{
	std::pair(".fx", [](std::string path, std::string name) { RESOURCES->Load<Effect>(path, name); }),
	std::pair(".mp3", [](std::string path, std::string name) { RESOURCES->Load<Sound>(path, name); }),
	std::pair(".wav", [](std::string path, std::string name) { RESOURCES->Load<Sound>(path, name); }),
	std::pair(".temd", [](std::string path, std::string name) { RESOURCES->Load<Model>(path, name); }),
	std::pair(".jpg", [](std::string path, std::string name) { RESOURCES->Load<Texture>(path, name); }),
	std::pair(".png", [](std::string path, std::string name) { RESOURCES->Load<Texture>(path, name); }),
};

void ResourceManager::Init(const std::string& dataPath)
{
	LOGGER->Log<LOG_INFO>("Initializing resource manager");

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
	for (auto& r : m_Resources)
		r.second.Cleanup();
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