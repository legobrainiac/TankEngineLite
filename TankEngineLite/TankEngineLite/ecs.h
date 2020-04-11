//////////////////////////////////////////////////////////////////////////
// Author: Tomas Antonio Sanches Pinto, 2020
// ecs.h: Entity Component System, version 2.1 as a separate module of 
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

#define INTE
#define ECS_LOG
#define DEBUG_POOL

// If inside tank engine, we can take the singleton dependencies
#ifdef INTE
#include "Singleton.h"
#include "Pool.h"
#include "Logger.h"
#endif 

namespace ECS
{

///////////////////////////////////////////
// Hierarchy of this ECS way of working

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
class System
{
public:
	virtual ~System() = default;
	inline virtual std::type_index GetSystemTypeAsComponent() = 0;

	inline virtual EntityComponent* PushComponent(Entity* pE) = 0;
	inline virtual void PopComponent(EntityComponent* pComp) = 0;

	inline virtual void Update(float dt) = 0;
	inline virtual void ForAll(std::function<void(EntityComponent*)> execFunc) = 0;
	inline virtual void ImGuiDebug() = 0;
};

struct SystemIdentifier
{
	System* pSystem;
	uint32_t systemId;
};

//////////////////////////////////////////////////////////////////////////
class EntityComponent
{
public:
	EntityComponent() : m_pOwner(nullptr), m_pSystem(nullptr) {}
	EntityComponent(Entity* pE) : m_pOwner(pE), m_pSystem(nullptr) {}
	virtual ~EntityComponent() = default;
	virtual void Update(float dt) { (void)dt; }
	inline Entity* GetOwner() { return m_pOwner; }

	// System
	inline void SetSystem(System* pS) { m_pSystem = pS; }
	inline System* GetSystem() { return m_pSystem; }

protected:
	Entity* m_pOwner;
	System* m_pSystem;
};

//////////////////////////////////////////////////////////////////////////
template<typename T, uint32_t C, uint32_t I, SystemExecutionStyle E>
class WorldSystem
	: public System
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
		T* pEc = m_pComponentPool->GetAndInit(pE);
		pEc->SetSystem(this); // This is not rly idea xd
		return pEc;
	}

	inline void PopComponent(EntityComponent* pComp) override
	{
		m_pComponentPool->Pop((T*)pComp);
	}

	inline void Update(float dt) override
	{
		m_pComponentPool->ForAllActive([&](T* pC)
			{
				pC->Update(dt);
			});
	}

	inline void ImGuiDebug() override
	{
#ifdef DEBUG_POOL
		std::string str = GetSystemTypeAsComponent().name();
		str = str.substr(6);

		if (ImGui::BeginTabItem(str.c_str()))
		{
			ImGui::Separator();
			m_pComponentPool->ImGuiDebugUi();
			ImGui::EndTabItem();
		}
#endif // DEBUG_POOL
	}

	inline void ForAll(std::function<void(EntityComponent*)> execFunc) override
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
#ifdef INTE
		Logger::GetInstance()->Log<LOG_SUCCESS>("World created");
#endif
#endif
	}

	~World();

	inline Entity* CreateEntity();
	inline void DestroyEntity(uint32_t id);

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
#ifdef INTE
		std::stringstream stream;
		stream << "Registered world system : " + std::string(typeIndex.name());
		Logger::GetInstance()->Log<LOG_INFO>(stream.str());
#endif
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

	void ImGuiDebug()
	{
#ifdef DEBUG_POOL
		std::string str = "World: " + std::to_string(m_ID) + " system debugger";
		ImGui::Begin(str.c_str());
		if (ImGui::BeginTabBar("World Systems", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("World stats"))
			{
				ImGui::Separator();

				// World Id
				ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "World id: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(m_ID).c_str());

				// System count
				ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Entity count: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(m_pEntities.size()).c_str());

				// Id counter
				ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "UID counter: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(m_IdCounter).c_str());

				// Entity count
				ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Component system count: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(m_Systems.size()).c_str());

				ImGui::EndTabItem();
			}
#endif
			for (auto system : m_Systems)
				system.second.pSystem->ImGuiDebug();
#ifdef DEBUG_POOL
			ImGui::EndTabBar();
		}
		ImGui::End();
#endif
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

	void CleanComponents()
	{
		for (auto comp : m_EntityComponents)
		{
			auto pComponent = comp.second;
			auto pSystem = pComponent->GetSystem();
			pSystem->PopComponent(pComponent);
		}

		m_EntityComponents.clear();
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
// World create entity and destroy entity declaration
inline Entity* World::CreateEntity()
{
	auto pEntity = new Entity(m_IdCounter, this);
	m_pEntities[m_IdCounter++] = pEntity;
	return pEntity;
}

inline void World::DestroyEntity(uint32_t id)
{
	auto entityIt = m_pEntities.find(id);

	if (entityIt != m_pEntities.end())
	{
		auto pEntity = *entityIt;
		pEntity.second->CleanComponents();
		delete pEntity.second;
		m_pEntities.erase(entityIt);
	}
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

	void ImGuiDebug()
	{
		for (auto world : m_Worlds)
			world.second->ImGuiDebug();
	}

	RO5(Universe);

private:
	uint32_t m_NextWorldIndex;
	std::map<uint32_t, World*> m_Worlds;
};

};

#endif // !ECS_H