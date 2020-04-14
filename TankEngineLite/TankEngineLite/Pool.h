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
    
    // Using this in the specific use case of ecs to initialized the object with it's parent entity
    template <typename INIT_TYPE>
    T* GetAndInit(INIT_TYPE* pParentObj)
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
    
	T* Get()
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
                    new (pFreeObject)T ();
                    
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
		uint32_t index = (uint32_t)(pPop - m_pPool);

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

	void Reset()
	{
		for (int i = 0; i < (S / 8); ++i)
			m_pLookUp[i] = 0;
		m_ActiveCount = 0;
	}

	// For all active and inactive
	void ForAll(const std::function<void(T*)>& f)
	{
		T* pPoolItem = m_pPool;

		for (int i = 0; i < S; ++i)
		{
			f(pPoolItem);
			pPoolItem++;
		}
	}

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
            if(*pBox == 0)
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
	T* m_pPool;
    char m_LookUpInternal[S / 8];
	char* m_pLookUp;
	uint32_t m_ActiveCount;

public:
	inline T* GetPool() { return m_pPool; }
	inline char* GetLookUp() { return m_pLookUp; }
	inline uint32_t GetActiveCount() { return m_ActiveCount; }

	void PrintLookUpTable()
	{
		std::cout << "Active count: " << GetActiveCount() << std::endl;

		char* pPoolLookUp = m_pLookUp;

		for (int i = 0; i < S / 8; ++i)
		{
			std::cout << std::bitset<8>(*pPoolLookUp) << std::endl;
			pPoolLookUp++;
		}

		std::cout << std::endl;
	}

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