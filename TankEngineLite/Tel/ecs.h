//////////////////////////////////////////////////////////////////////////
// Author: Tomas Antonio Sanches Pinto, 2019
// ecs.h: Entity Component System, version 2.0 as a separate module of 
//		TheTankEngine3D and TankEngineMini
//////////////////////////////////////////////////////////////////////////

#ifndef ECS_H
#define ECS_H

#include <unordered_map>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <tuple>

typedef unsigned int UINT;

namespace ECS
{

class Entity;
class WorldSystem;
class EntityComponent;

//////////////////////////////////////////////////////////////////////////
class EntityComponent
{
public:
	EntityComponent(ECS::Entity* pE) : m_pOwner(pE) {}
	virtual ~EntityComponent() {}

    virtual void Update(float dt) { (void)dt; }
	inline Entity* GetOwner() { return m_pOwner; }

protected:
	Entity* m_pOwner;
};

//////////////////////////////////////////////////////////////////////////
class Entity
{
public:
	Entity(UINT id, WorldSystem* pWorld)
		: m_ID(id),
		m_pWorld(pWorld),
		m_EntityComponents{}
	{ }

	~Entity()
	{
		for (auto& c : m_EntityComponents)
			delete c.second;
	}

	//////////////////////////////////////////////////////////////////////////
	/// Pushing components
	//////////////////////////////////////////////////////////////////////////
private:
	template<typename T>
	T* PushComponentImpl()
	{
		auto typeIndex = std::type_index(typeid(T));

		// Only create a new component o type T if none other was found
		if (m_EntityComponents.find(typeIndex) == m_EntityComponents.end())
			m_EntityComponents[typeIndex] = static_cast<EntityComponent*>(new T(this));

		return static_cast<T*>(m_EntityComponents[typeIndex]);
	}

public:
	template<typename... T>
	inline std::tuple<T* ...> PushComponents()
	{
		return std::tuple<T * ...>(PushComponentImpl<T>()...);
	}

	//////////////////////////////////////////////////////////////////////////
	/// Popping components 
	//////////////////////////////////////////////////////////////////////////
private:
	template<typename T>
	T* PopComponentImpl()
	{
		auto typeIndex = std::type_index(typeid(T));
		auto found = m_EntityComponents.find(typeIndex);

		if (found != m_EntityComponents.end())
		{
			delete found->second;
			auto it = m_EntityComponents.erase(typeIndex);
		}

		return nullptr;
	}

public:
	template<typename... T>
	inline void PopComponents()
	{
		std::tuple<T* ...> t{ PopComponentImpl<T>()... };
	}

	//////////////////////////////////////////////////////////////////////////
	/// Getting component handles
	//////////////////////////////////////////////////////////////////////////
public:
	template<typename T>
	T* GetComponent()
	{
		auto typeIndex = std::type_index(typeid(T));
		auto found = m_EntityComponents.find(typeIndex);

		if (found != m_EntityComponents.end())
			return static_cast<T*>(found->second);

		return nullptr;
	}

	template<typename... T>
	inline std::tuple<T* ...> GetComponents()
	{
		return std::tuple<T * ...> { GetComponent<T>()... };
	}

public:
	void UpdateComponents(float dt)
	{
		for (auto& e : m_EntityComponents)
			e.second->Update(dt);
	}

	inline size_t GetComponentCount() const { return m_EntityComponents.size(); }
	inline UINT GetID() const { return m_ID; }

	inline WorldSystem* GetWorld() { return m_pWorld; }

protected:
	const UINT m_ID;
	WorldSystem* m_pWorld;
	std::unordered_map<std::type_index, EntityComponent*> m_EntityComponents;
};

//////////////////////////////////////////////////////////////////////////
class WorldSystem
{
public:
	WorldSystem()
		: m_pEntities{}
		, m_IdCounter{ }
	{}

	~WorldSystem()
	{
		for (auto& e : m_pEntities)
			delete e.second;
	}

	Entity* CreateEntity()
	{
		auto pEntity = new Entity(m_IdCounter, this);

		m_pEntities[m_IdCounter] = pEntity;
		m_IdCounter++;

		return m_pEntities[m_IdCounter - 1];
	}

	void DestroyEntity(UINT id)
	{
		m_DeleteBuffer.push_back(id);
	}

	void Update(float dt)
	{
		// Update world
		for (auto& e : m_pEntities)
			e.second->UpdateComponents(dt);

		// Process deletions
		for (UINT id : m_DeleteBuffer)
		{
			auto foundEntity = m_pEntities.find(id);

			if (foundEntity != m_pEntities.end())
			{
				delete foundEntity->second;
				m_pEntities.erase(foundEntity);
			}
		}

		m_DeleteBuffer.clear();
	}

	template<typename T>
	void ForAll(std::function<void(T*)> action)
	{
		for (auto& e : m_pEntities)
		{
			T* pComponent = e.second->GetComponent<T>();

			if (pComponent)
				action(pComponent);
		}
	}

	template<typename T>
	std::vector<T*> GetAll()
	{
		std::vector<T*> components;

		for (auto& e : m_pEntities)
		{
			T* pComponent = e.second->GetComponent<T>();

			if (pComponent)
				components.push_back(pComponent);
		}

		return components;
	}

	Entity* GetEntity(UINT id)
	{
		auto foundEntity = m_pEntities.find(id);

		if (foundEntity != m_pEntities.end())
			return foundEntity->second;

		return nullptr;
	}

private:
	std::unordered_map<UINT, Entity*> m_pEntities;
	std::vector<UINT> m_DeleteBuffer;
	UINT m_IdCounter;
};

}

#endif