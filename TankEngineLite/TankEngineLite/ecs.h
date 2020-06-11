//////////////////////////////////////////////////////////////////////////
// Author: Tomas Antonio Sanches Pinto, 2020
// ecs.h: Entity Component System, version 2.1 as a separate module of 
//		TheTankEngine3D and TankEngineMini
//////////////////////////////////////////////////////////////////////////

#ifndef ECS_H
#define ECS_H

#include <memory_resource>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <algorithm>
#include <typeinfo>
#include <thread>
#include <future>
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
#include "MemoryTracker.h"
#include "Profiler.h"
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
enum ExecutionStyle { SYNCHRONOUS, ASYNCHRONOUS, DYNAMIC, NOEXEC };
template<typename T, uint32_t C, uint32_t I, ExecutionStyle E>
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
	
	inline virtual std::type_index GetSystemTypeAsComponent() const = 0;
	inline virtual ExecutionStyle GetExecutionStyle() const = 0;

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
	constexpr EntityComponent() : m_pOwner(nullptr), m_pColliderSystem(nullptr) {}
	constexpr EntityComponent(Entity* pE) : m_pOwner(pE), m_pColliderSystem(nullptr) {}

	virtual ~EntityComponent() = default;
	virtual void Update([[maybe_unused]] float dt) { }

	// System
	inline void SetSystem(System* pS) { m_pColliderSystem = pS; }
	
	[[nodiscard]] constexpr auto GetOwner() const noexcept -> Entity* { return m_pOwner; }
	[[nodiscard]] constexpr auto GetSystem() const noexcept -> System* { return m_pColliderSystem; }

protected:
	Entity* m_pOwner;
	System* m_pColliderSystem;
};

//////////////////////////////////////////////////////////////////////////
template<typename T, uint32_t C, uint32_t I, ExecutionStyle E>
class WorldSystem
	: public System
{
public:
	inline WorldSystem()
		: m_ID(I)
		, m_ExecutionStyle(E)
	{
		m_pComponentPool = new (Memory::New<Pool<T, C>>()) Pool<T, C>();
	}

	inline ~WorldSystem() override
	{
		Memory::Delete(m_pComponentPool);
	}

	// System identification
	[[nodiscard]] constexpr auto GetSystemTypeAsComponent() const noexcept -> std::type_index override { return std::type_index(typeid(T)); }
	[[nodiscard]] constexpr auto GetSystemAsId() const noexcept -> uint32_t { return I; }
	[[nodiscard]] constexpr auto GetExecutionStyle() const noexcept -> ExecutionStyle override { return m_ExecutionStyle; };

	// System management
	inline EntityComponent* PushComponent(Entity* pE) override
	{
		const auto pEc = m_pComponentPool->GetAndInit(pE);
		pEc->SetSystem(this); // This is not rly idea xd
		return pEc;
	}

	inline void PopComponent(EntityComponent* pComp) override
	{
		m_pComponentPool->Pop(static_cast<T*>(pComp));
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
	ExecutionStyle m_ExecutionStyle;
	Pool<T, C>* m_pComponentPool;
};

//////////////////////////////////////////////////////////////////////////
class World
{
public:
	World(uint32_t givenId)
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

	[[nodiscard]] Entity* CreateEntity();
	void AsyncCreateEntity(std::function<void(Entity*)> initializer)
	{
		// TODO(tomas): not thread safe
		m_AsyncCreationBuffer.push_back(initializer);
	}

	inline void DestroyEntity(uint32_t id);
	inline void AsyncDestroyEntity(uint32_t id)
	{
		// TODO(tomas): not thread safe
		m_AsyncDestroyBuffer.push_back(id);
	}

	//////////////////////////////////////
	// Push Systems impl
private:
	template<typename T>
	constexpr T* PushSystem()
	{
		auto typeIndex = std::type_index(typeid(T));

		// Create world
		auto pWorldSystem = new (Memory::New<T>()) T();

		// Create system identifier
		SystemIdentifier system;
		system.pSystem = static_cast<System*>(pWorldSystem);
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
	constexpr std::tuple<T* ...> PushSystems()
	{
		return std::tuple<T * ...> { PushSystem<T>()... };
	}

public:
	template<typename T>
	[[nodiscard]] constexpr System* GetSystemByComponent() const
	{
		// Find system of type T
		auto type = std::type_index(typeid(T));
		auto found = std::find_if(m_Systems.begin(), m_Systems.end(),
			[&](auto sys)
			{
				return (sys.second.pSystem->GetSystemTypeAsComponent() == type);
			});

		System* pS = nullptr;

		// If found, return it
		if (found != m_Systems.end())
			pS = found->second.pSystem;

		return pS;
	}

public:
	void Update(float dt)
	{
		std::vector<std::future<void>> futures{};

		// TODO(tomas): Each async system owns a thread that is waiting for 
		//  the go ahead signal to do another loop
		//  Right now, we create an async ting every time, not good 
		// TODO(tomas): ExecutionStyle::DYNAMIC, let ECS decide, at runtime
		//  which execution policy it should take
		
		// Async system threads launched
		for (auto system : m_Systems)
		{
			const auto pSystem = system.second.pSystem;

			if (pSystem->GetExecutionStyle() == ExecutionStyle::ASYNCHRONOUS)
				futures.push_back(std::async(std::launch::async, &System::Update, pSystem, dt));
		}

		// Do normal updating
		for (auto system : m_Systems)
		{
			const auto pSystem = system.second.pSystem;

			if (pSystem->GetExecutionStyle() == ExecutionStyle::SYNCHRONOUS)
				pSystem->Update(dt);
		}

		// Cleanup futures
		Profiler::GetInstance()->BeginSubSession<SESSION_THREAD_WAITING>();
		for (auto& f : futures)
			f.get();
		Profiler::GetInstance()->EndSubSession();

		// Process async destructions
		for (uint32_t i : m_AsyncDestroyBuffer)
			DestroyEntity(i);

		// Process async insertions
		for (auto& asyncCreator : m_AsyncCreationBuffer)
			asyncCreator(CreateEntity());

		m_AsyncDestroyBuffer.clear();
		m_AsyncCreationBuffer.clear();
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

	std::vector<uint32_t> m_AsyncDestroyBuffer;
	std::vector<std::function<void(Entity*)>> m_AsyncCreationBuffer;
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
	constexpr std::tuple<T* ...> PushComponents()
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
	constexpr void PopComponents()
	{
		std::tuple<T* ...> t{ PopComponentImpl<T>()... };
	}

	//////////////////////////////////////////////////////////////////////////
	/// Getting component handles
	//////////////////////////////////////////////////////////////////////////
public:
	template<typename T>
	[[nodiscard]] T* GetComponent()
	{
		auto typeIndex = std::type_index(typeid(T));
		auto found = m_EntityComponents.find(typeIndex);

		if (found != m_EntityComponents.end())
			return static_cast<T*>(found->second);

		return nullptr;
	}

	template<typename... T>
	[[nodiscard]] constexpr std::tuple<T* ...> GetComponents()
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
	[[nodiscard]] inline auto GetComponentCount() const noexcept -> size_t { return m_EntityComponents.size(); }
	[[nodiscard]] constexpr auto GetId() const noexcept -> uint32_t { return m_ID; }
	[[nodiscard]] constexpr auto GetWorld() const noexcept -> World* { return m_pWorld; }

	constexpr auto SetTag(uint32_t tag) noexcept { m_Tag = tag; }
	[[nodiscard]] constexpr auto GetTag() const noexcept -> uint32_t { return m_Tag; }

private:
	const uint32_t m_ID;
	uint32_t m_Tag;
	World* m_pWorld;
	std::unordered_map<std::type_index, EntityComponent*> m_EntityComponents;
};

//////////////////////////////////////////////////////////////////////////
// World create entity and destroy entity declaration
inline Entity* World::CreateEntity()
{
	auto pEntity = new(Memory::New<Entity>()) Entity(m_IdCounter, this);
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
		Memory::Delete(pEntity.second);
		m_pEntities.erase(entityIt);
	}
}

//////////////////////////////////////////////////////////////////////////
// World destructor declaration
inline World::~World()
{
	for (auto system : m_Systems)
		Memory::Delete(system.second.pSystem);

	for (auto entity : m_pEntities)
		Memory::Delete(entity.second);
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
			Memory::Delete<World>(world.second);
	}

	inline World* PushWorld()
	{
		uint32_t id = m_NextWorldIndex++;

		auto pWorld = new(Memory::New<World>()) World(id);
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