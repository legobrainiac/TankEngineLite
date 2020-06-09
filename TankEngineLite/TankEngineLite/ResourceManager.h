#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Singleton.h"
#include "d3dx11effect.h"

#include <unordered_map>
#include <type_traits>
#include <functional>
#include <string_view>
#include <typeindex>
#include <map>

#include "MemoryTracker.h"
#include "Logger.h"

#define RESOURCES ResourceManager::GetInstance()

class Texture;
class Font;

class IResource
{
public:
	virtual bool Initialize([[maybe_unused]] const std::string& path) { return false; };
	virtual void Shutdown() {}
};

using TypeResolver = std::function<void(std::string, std::string)>;

struct ResourceDescriptor
{
	ResourceDescriptor()
		: resourceType(std::type_index(typeid(void)))
		, pResource(nullptr)
	{
	}

	std::type_index resourceType;
	IResource* pResource;

	template<
		typename T, 
		typename = std::enable_if_t<std::is_base_of_v<IResource, T>>
	>
	[[nodiscard]] T* DescribedAs()
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
	void Init(const std::string& data);
	void Destroy();

	void LoadAllInFolder();

	void LoadDecode([[maybe_unused]] const std::string& path)
	{
		const auto lastDot = path.find_last_of('.');
		const auto lastSlash = path.find_last_of('\\');
		
		if (lastDot == -1)
			return;

		auto resourceType = path.substr(lastDot);
		resourceType = resourceType.substr(0, resourceType.size() - 1);

		auto resourceName = path.substr(lastSlash + 1, (lastDot - lastSlash) - 1);

		// Figure out if a resolver exists
		const auto it = TypeResolvers.find(resourceType);
		if (it != TypeResolvers.cend())
			it->second(resourceName + resourceType, resourceName);
	}

	template<
		typename T,
		typename = std::enable_if_t<std::is_base_of_v<IResource, T>>
	>
	T* Load(const std::string& file, const std::string& name)
	{
		// If texture already exists, we don't do anything, just return it
		const auto it = m_Resources.find(name);

		if (it != m_Resources.cend())
			return it->second.DescribedAs<T>();

		// Otherwise
		const auto fullPath = m_DataPath + file;
		auto pResource = new (Memory::New<T>()) T();

		try
		{
			bool result = pResource->Initialize(fullPath.c_str());

			if (pResource == nullptr || !result)
				throw std::runtime_error(std::string("Failed to load: " + file + " as " + name));

			ResourceDescriptor rd{};
			rd.resourceType = std::type_index(typeid(T));
			rd.pResource = static_cast<IResource*>(pResource);

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

	template<
		typename T,
		typename = std::enable_if_t<std::is_base_of_v<IResource, T>>
	>
	[[nodiscard]] T* Get(const std::string& name)
	{
		const auto it = m_Resources.find(name);

		if (it != m_Resources.cend())
			return it->second.DescribedAs<T>();

		Logger::GetInstance()->Log<LOG_WARNING>("Failed to locate -> ", name);

		return nullptr;
	}

	static void AddTypeResolver(const std::pair<std::string_view, TypeResolver>& resolver)
	{
		TypeResolvers[resolver.first] = resolver.second;
	}

private:
	friend class Singleton<ResourceManager>;
	ResourceManager() = default;

	std::string m_DataPath;
	std::map<std::string, ResourceDescriptor> m_Resources;

	// Resource resolution
	static std::map<std::string_view, TypeResolver> TypeResolvers;
};
#endif // !RESOURCE_MANAGER_H
