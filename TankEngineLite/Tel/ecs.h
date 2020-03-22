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
#include <algorithm>
#include <typeinfo>
#include <vector>
#include <tuple>
#include <map>

#ifdef ECS_LOG
#include <iostream>
#include <string>
#endif

#define RO5(CLASS_NAME)\
CLASS_NAME(const CLASS_NAME&) = delete;\
CLASS_NAME(CLASS_NAME&&) = delete;\
CLASS_NAME& operator=(const CLASS_NAME&) = delete;\
CLASS_NAME& operator=(CLASS_NAME&&) = delete;

#define _(T) (void)T // Unused annoyances

// If inside tank engine, we can take the singleton dependencies
#ifdef INTE
#include "Singleton.h"
#include "Pool.h"
#else // otherwise we create our own
template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		static T pInstance{};
		return &pInstance;
	}

	virtual ~Singleton() = default;
	RO5(Singleton);

protected:
	Singleton() = default;
};

// TODO(tomas): self contain the pool
#endif 

namespace ECS
{

///////////////////////////////////////////
// Hiearchy of this ecs way of working

// Universe contains all the worlds
class Universe;

// World contains all the world systems and Entities
class World;

// Contains all type of one Entity Component
enum SystemExecutionStyle { SYNCHRONOUS, ASYNCHRONOUS };
template<typename T, uint32_t C, uint32_t I, SystemExecutionStyle E>
class WorldSystem;

// An entity inside a world
class Entity;

// A component that belong to an entity and lives in a WorldSystem
class EntityComponent;

//////////////////////////////////////////////////////////////////////////
class EntityComponent
{
public:
	EntityComponent() {}
	EntityComponent(Entity* pE) : m_pOwner(pE) {}
	virtual void CleanInitialize(Entity* pE) { m_pOwner = pE; }
	virtual ~EntityComponent() {}

	virtual void Update(float dt) { (void)dt; }
	inline Entity* GetOwner() { return m_pOwner; }

	inline void SetDirty(bool val) { m_IsDirty = val; }

protected:
	Entity* m_pOwner;
	bool m_IsDirty = true;
};

//////////////////////////////////////////////////////////////////////////
class System
{
public:
	virtual ~System() {}
	inline virtual std::type_index GetSystemTypeAsComponent() = 0;

	inline virtual EntityComponent* PushComponent(Entity* pE) = 0;
	inline virtual void PopComponent(EntityComponent* pComp) = 0;

	inline virtual void Update(float dt) = 0;
	inline virtual void ForAll(std::function<void(EntityComponent*)> execFunc) = 0;
};

struct SystemIdentifier
{
	System* pSystem;
	uint32_t systemId;
};

template<typename T, uint32_t C, uint32_t I, SystemExecutionStyle E>
class WorldSystem : public System
{
public:
	inline WorldSystem()
		: m_ID(I)
	{
		m_pComponentPool = new Pool<T, C>();
	}

	inline ~WorldSystem() override
	{
		delete m_pComponentPool;
	}

	// System identification
	inline std::type_index GetSystemTypeAsComponent() override
	{
		return std::type_index(typeid(T));
	}

	inline uint32_t GetSystemAsId()
	{
		return I;
	}

	// System management
	inline EntityComponent* PushComponent(Entity* pE) override
	{
		T* pEc = m_pComponentPool->Get();
		pEc->CleanInitialize(pE);
        pEc->SetDirty(false);
		return pEc;
	}

	inline void PopComponent(EntityComponent* pComp) override
	{
		m_pComponentPool->Pop((T*)pComp);
		pComp->SetDirty(true);
	}

	inline virtual void Update(float dt) override
	{
		m_pComponentPool->ForAllActive([&](T* pC)
			{
				pC->Update(dt);
			});
	}

	inline virtual void ForAll(std::function<void(EntityComponent*)> execFunc) override
	{
		m_pComponentPool->ForAllActive([&](T* pC)
			{
				execFunc(pC);
			});
	}

private:
	uint32_t m_ID;
	Pool<T, C>* m_pComponentPool;
};

//////////////////////////////////////////////////////////////////////////
class World
{
public:
	inline World(uint32_t givenId)
		: m_ID(givenId)
		, m_IdCounter(0)
	{
#ifdef ECS_LOG
		std::cout << "World create" << std::endl;
#endif
	}

	~World();

	inline Entity* CreateEntity();

	//////////////////////////////////////
	// Push Systems impl
private:
	template<typename T>
	inline T* PushSystem()
	{
		auto typeIndex = std::type_index(typeid(T));

		// Create world
		auto pWorldSystem = new T();

		// Create system identifier
		SystemIdentifier system;
		system.pSystem = (System*)pWorldSystem;
		system.systemId = pWorldSystem->GetSystemAsId();

		// Store it linked to the EntityComponent type of the system
		m_Systems[pWorldSystem->GetSystemTypeAsComponent()] = system;

#ifdef ECS_LOG
		std::cout << "Registered world system : " + std::string(typeIndex.name()) << std::endl;
#endif  
		return pWorldSystem;
	}

public:
	template<typename... T>
	inline std::tuple<T* ...> PushSystems()
	{
		return std::tuple<T * ...> { PushSystem<T>()... };
	}

public:
	template<typename T>
	System* GetSystemByComponent()
	{
		auto type = std::type_index(typeid(T));
		auto found = std::find_if(m_Systems.begin(), m_Systems.end(),
			[&](auto sys)
			{
				return (sys.second.pSystem->GetSystemTypeAsComponent() == type);
			});

		System* pS = nullptr;

		if (found != m_Systems.end())
			pS = found->second.pSystem;

		return pS;
	}

public:
	void Update(float dt/*CommandChain/SignalChain?*/)
	{
		for (auto system : m_Systems)
			system.second.pSystem->Update(dt);
	}

private:
	uint32_t m_ID;
	uint32_t m_IdCounter;
	std::unordered_map<uint32_t, Entity*> m_pEntities;
	std::unordered_map<std::type_index, SystemIdentifier> m_Systems;
};

//////////////////////////////////////////////////////////////////////////
class Entity
{
public:
	Entity(uint32_t id, World* pWorld)
		: m_ID(id)
		, m_pWorld(pWorld)
		, m_EntityComponents()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	/// Pushing components
	//////////////////////////////////////////////////////////////////////////
private:
	template<typename T>
	T* PushComponentImpl()
	{
		auto typeIndex = std::type_index(typeid(T));
		EntityComponent* pC = nullptr;

		// Only create a new component o type T if none other was found
		if (m_EntityComponents.find(typeIndex) == m_EntityComponents.end())
		{
			// Get the corresponding component world system
			auto pCWS = m_pWorld->GetSystemByComponent<T>();

			pC = pCWS->PushComponent(this);
			m_EntityComponents[typeIndex] = pC;

			return static_cast<T*>(pC);
		}

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
			// Get the corresponding component world system
			auto pCWS = m_pWorld->GetSystemByComponent<T>();
			pCWS->PopComponent((*found).second);
			m_EntityComponents.erase(typeIndex);
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
	inline size_t GetComponentCount() const { return m_EntityComponents.size(); }
	inline uint32_t GetId() const { return m_ID; }
	inline World* GetWorld() { return m_pWorld; }

private:
	const uint32_t m_ID;
	World* m_pWorld;
	std::unordered_map<std::type_index, EntityComponent*> m_EntityComponents;
};

//////////////////////////////////////////////////////////////////////////
// World create entity declaration
inline Entity* World::CreateEntity()
{
	auto pEntity = new Entity(m_IdCounter, this);
	m_pEntities[m_IdCounter++] = pEntity;
	return pEntity;
}

//////////////////////////////////////////////////////////////////////////
// World destructor declaration
inline World::~World()
{
	for (auto system : m_Systems)
		delete system.second.pSystem;

	for (auto entity : m_pEntities)
		delete entity.second;
}

//////////////////////////////////////////////////////////////////////////
class Universe
	: public Singleton<Universe>
{
public:
	inline Universe()
		: m_Worlds()
		, m_NextWorldIndex(0U)
	{
	}

	inline ~Universe()
	{
		for (auto world : m_Worlds)
			delete world.second;
	}

	inline World* PushWorld()
	{
		uint32_t id = m_NextWorldIndex++;

		auto pWorld = new World(id);
		m_Worlds[id] = pWorld;

		return pWorld;
	}

	void Update(float dt)
	{
		for (auto world : m_Worlds)
			world.second->Update(dt);
	}

	RO5(Universe);

private:
	uint32_t m_NextWorldIndex;
	std::map<uint32_t, World*> m_Worlds;
	friend class Singleton<Universe>;
};

};

#endif