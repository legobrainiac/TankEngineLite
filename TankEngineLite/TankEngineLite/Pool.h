//////////////////////////////////////////////////////////////////////////
// Author: Tomas Antonio Sanches Pinto, 2019
// pool.h: Memory pool, version 2.0 as a separate module of 
//		TheTankEngine3D and TankEngineMini
//////////////////////////////////////////////////////////////////////////

#ifndef POOL_H
#define POOL_H

#include <type_traits>
#include <functional>
#include <iostream>
#include <bitset>

// #define POOL_NO_THROW
// Thank you DragonSlayer0531#3017 for the help with SFINAE

#ifdef DEBUG_POOL
#include <sstream>
#include "imgui.h"
#include "MemoryTracker.h"
#endif // DEBUG_POOL

template <uint32_t L, uint32_t R>
struct constexpr_mod
{
	static constexpr uint32_t value = L % R;
};

//////////////////////////////////////////////////////////////////////////
// Class: Pool<typename T, uint32_s S>
// Description: Memory pool container that generates a block of memory of sizeof(T) * S,
//  to the user, the memory pool behaves as an T[S] array
template<
	typename T,
	uint32_t S,
	std::enable_if_t<constexpr_mod<S, 8>::value == 0, int> = 0
>
class Pool
{
public:
	Pool()
	{
		m_pPool = Memory::New<T>(S);
		m_pLookUp = &m_LookUpInternal[0];
		m_ActiveCount = 0;
	}

	~Pool()
	{
		// Call destructor on the still active pool elements, 
		//  We don't want to deallocate just yet
		ForAllActive([](T* pObj) { pObj->~T(); });

		// Free the memory block 
		Memory::Delete(m_pPool, false);
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetAndInit
	// FullName:  Pool::GetAndInit<typename INIT_TYPE>
	// Access:    public 
	// Returns:   constexpr T*
	// Description: Get and initialize an object from the pool
	// Note: Using this in the specific use case of ECS to initialized the object with it's parent entity
	// Parameter: INIT_TYPE* pParentObj
	template <typename INIT_TYPE>
	[[nodiscard]] constexpr T* GetAndInit(INIT_TYPE* pParentObj)
	{
		// Test if pool is full
		if (m_ActiveCount + 1 > S)
		{
#ifndef POOL_NO_THROW
 			throw std::exception("Pool is full!");
#else
			return nullptr;
#endif
		}

		char* pPoolLookUp = m_pLookUp;
		for (int i = 0; i < S / 8; ++i)
		{
			// Check pool map for inactive entities
			for (int j = 0; j < 8; ++j)
			{
				char flag = (1 << j);

				// Check inverse byte against flag
				if (~(*pPoolLookUp) & flag)
				{
					*pPoolLookUp |= flag;
					m_ActiveCount++;

					// Placement if to get an initialized object
					T* pFreeObject = &m_pPool[i * 8 + j];
                    new (pFreeObject)T (pParentObj);
					return pFreeObject;
				}
			}

			pPoolLookUp++;
		}

		// If allocation failed at this point, maybe some other thread added to the pool
#ifndef POOL_NO_THROW
		throw std::exception("Pool is full!");
#else
		return nullptr;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Get
	// FullName:  Pool<T, S>::Get
	// Access:    public 
	// Returns:   constexpr T*
	// Description: Get an available object from the pool
	[[nodiscard]] constexpr T* Get()
	{
		// Test if pool is full
		if (m_ActiveCount + 1 > S)
		{
#ifndef POOL_NO_THROW
			throw std::exception("Pool is full!");
#else
			return nullptr;
#endif
		}

		char* pPoolLookUp = m_pLookUp;
		for (int i = 0; i < S / 8; ++i)
		{
			// Check pool map for inactive entities
			for (int j = 0; j < 8; ++j)
			{
				char flag = (1 << j);

				// Check inverse byte against flag
				if (~(*pPoolLookUp) & flag)
				{
					*pPoolLookUp |= flag;
					m_ActiveCount++;

					// Placement if to get an initialized object
					T* pFreeObject = &m_pPool[i * 8 + j];
					new (pFreeObject)T();
					return pFreeObject;
				}
			}

			pPoolLookUp++;
		}

		// If allocation failed at this point, maybe some other thread added to the pool
#ifndef POOL_NO_THROW
		throw std::exception("Pool is full!");
#else
		return nullptr;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Pop
	// FullName:  Pool<T, S>::Pop
	// Access:    public 
	// Returns:   void
	// Description: Pop and object from the pool
	// Parameter: T * pPop
	void Pop(T* pPop)
	{
		if (pPop < m_pPool || pPop > m_pPool + S)
		{
#ifndef	POOL_NO_THROW
			throw std::exception("Address out of bounds of pool!");
#else
			return;
#endif
		}

		// Get index inside pool
		uint32_t index = static_cast<uint32_t>(pPop - m_pPool);

		// Get box in look up table
		char* pBox = &m_pLookUp[(index / 8) % S];
		int boxOffset = index % 8;

		// If item active
		char flag = (1 << boxOffset);
		if (*pBox & flag)
		{
			// We manually call the destructor, we don't want to actually deallocate
			m_pPool[index].~T();

			*pBox ^= flag;
			m_ActiveCount--;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Reset
	// FullName:  Pool<T, S>::Reset
	// Access:    public 
	// Returns:   void
	// Description: Reset the pool to an empty state
	void Reset()
	{
		for (int i = 0; i < (S / 8); ++i)
			m_pLookUp[i] = 0;

		m_ActiveCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    ForAll
	// FullName:  Pool<T, S>::ForAll
	// Access:    public 
	// Returns:   void
	// Description: For all active and inactive objects, f(pObj)
	// Parameter: const std::function<void(T*)>& f
	void ForAll()
	{
		T* pPoolItem = m_pPool;

		for (int i = 0; i < S; ++i)
		{
			f(pPoolItem);
			pPoolItem++;
		}
	}

	//************************************
	// Method:    ForAllActive
	// FullName:  Pool<T, S>::ForAllActive
	// Access:    public 
	// Returns:   void
	// Description: For all active objects in the pool
	// Parameter: const std::function<void(T*)>& f
	void ForAllActive(const std::function<void(T*)>& f)
	{
		T* pPoolItem = m_pPool;

		for (int i = 0; i < S; ++i)
		{
			// Get box in look up table
			char* pBox = &m_pLookUp[(i / 8) % S];

#if 0 // This is broken, fix in ecs repo           
			// If the box is 0, then no item is active
			// TODO(tomas): unittest this
			if (*pBox == 0)
				continue;
#endif

			int boxOffset = i % 8;

			// If item active
			char flag = (1 << boxOffset);
			if (*pBox & flag)
				f(pPoolItem);

			pPoolItem++;
		}
	}

private:
	T* m_pPool = nullptr;
	char m_LookUpInternal[S / 8]{};
	char* m_pLookUp = nullptr;
	uint32_t m_ActiveCount = 0U;

public:

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetPool
	// FullName:  Pool<T, S>::GetPool
	// Access:    public 
	// Returns:   constexpr T*
	// Qualifier: const noexcept
	// Description: Get a pointer to the start of the pool
	[[nodiscard]] constexpr auto GetPool() const noexcept -> T* { return m_pPool; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetLookUp
	// FullName:  Pool<T, S>::GetLookUp
	// Access:    public 
	// Returns:   constexpr char*
	// Qualifier: const noexcept
	// Description: Get a pointer to the start of the lookup bit table
	[[nodiscard]] constexpr auto GetLookUp() const noexcept -> char* { return m_pLookUp; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetActiveCount
	// FullName:  Pool<T, S>::GetActiveCount
	// Access:    public 
	// Returns:   constexpr uint32_t
	// Qualifier: const noexcept
	// Description: Get active amount of items from the pool
	[[nodiscard]] constexpr auto GetActiveCount() const noexcept -> uint32_t { return m_ActiveCount; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    ImGuiDebugUi
	// FullName:  Pool<T, S>::ImGuiDebugUi
	// Access:    public 
	// Returns:   void
	// Description: Draw a Debug Card for this memory pool
	void ImGuiDebugUi()
	{
		std::stringstream stream;
		char* pPoolLookUp = m_pLookUp;
		int inc = 1;
		for (int i = 0; i < S / 8; ++i, inc++)
		{
			stream << std::bitset<8>(*pPoolLookUp) << " ";
			pPoolLookUp++;

			if (inc > 7)
			{
				stream << std::endl;
				inc = 0;
			}
		}
		ImGui::Text(stream.str().c_str());
		ImGui::SameLine();
	}
};

#endif // !POOL_H