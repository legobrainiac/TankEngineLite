#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Singleton.h"
#include "d3dx11effect.h"

#include <unordered_map>
#include <typeindex>
#include <map>

#include "SoundManager.h"
#include "MemoryTracker.h"
#include "Logger.h"

#define RESOURCES ResourceManager::GetInstance()

class Texture;
class Font;

class IShutdown
{
public:
	virtual void Shutdown() {}
};

struct ResourceDescriptor
{
	ResourceDescriptor()
		: resourceType(std::type_index(typeid(void)))
		, pResource(nullptr)
	{
	}

	std::type_index resourceType;
	IShutdown* pResource;

	template<typename T>
	T* DescribedAs()
	{
		if (std::type_index(typeid(T)) != resourceType)
		{
			LOGGER->Log<LOG_ERROR>("Described type != to existing type");
			return nullptr;
		}

		return static_cast<T*>(pResource);
	}

	void Cleanup()
	{
		pResource->Shutdown();
		Memory::Delete(pResource);
	}
};

class ResourceManager final 
	: public Singleton<ResourceManager>
{
public:
	void Init(const std::string& data, const std::wstring& dataW);
	void Destroy();

	template<typename T>
	T* Load(const std::string& file, const std::string& name)
	{
		// If texture already exists, we don't do anything, just return it
		const auto it = m_Resources.find(name);

		if (it != m_Resources.cend())
			return it->second.DescribedAs<T>();

		// Otherwise
		const auto fullPath = m_DataPath + file;
		auto pResource = Memory::New<T>();
		new (pResource) T();

		try
		{
			bool result = pResource->Initialize(fullPath.c_str());

			if (pResource == nullptr || !result)
				throw std::runtime_error(std::string("Failed to load: " + file + " as " + name));

			ResourceDescriptor rd{};
			rd.resourceType = std::type_index(typeid(T));
			rd.pResource = static_cast<IShutdown*>(pResource);

			m_Resources[name] = rd;
		}
		catch (const std::exception& e)
		{
			Logger::GetInstance()->Log<LOG_ERROR>(e.what());

			// Fall back exception, 
			//  we don't want memory leaks on failed texture creation
			Memory::Delete(pResource);

			return nullptr;
		}

		LOGGER->Log<LOG_SUCCESS>("Loaded ", name);
		return pResource;
	}

	template<typename T>
	T* Get(const std::string& name)
	{
		const auto it = m_Resources.find(name);

		if (it != m_Resources.cend())
			return it->second.DescribedAs<T>();

		Logger::GetInstance()->Log<LOG_WARNING>("Failed to locate -> ", name);

		return nullptr;
	}

	// TODO(tomas): Write wrappers around effect and sound so that they can 
	//  also be used with the generalized loader

	//////////////////////////////////////////////////////////////////////////
	// Effects
	ID3DX11Effect* LoadEffect(const std::wstring& file, const std::string& name);
	ID3DX11Effect* GetEffect(const std::string& name) const;

	//////////////////////////////////////////////////////////////////////////
	// Sound
	FMOD::Sound* LoadSound(const std::string& file, const std::string& name);
	FMOD::Sound* GetSound(const std::string& name) const;

private:
	friend class Singleton<ResourceManager>;
	ResourceManager() = default;

	std::string m_DataPath;
	std::wstring m_DataPathW;

	// Resources
	std::unordered_map<std::string, ID3DX11Effect*> m_pEffects;
	std::unordered_map<std::string, FMOD::Sound*> m_pSounds;

	std::map<std::string, ResourceDescriptor> m_Resources;
};
#endif // !RESOURCE_MANAGER_H
