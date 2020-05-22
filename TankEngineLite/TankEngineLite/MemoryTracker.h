#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#include <cmath>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// Struct: MemoryStatus
// Description: contains a description of the current memory status
struct MemoryStatus
{
	uint32_t m_TotalMemoryCount;
	uint32_t m_MemoryChunckCount;
};

//////////////////////////////////////////////////////////////////////////
// Class: Memory
// Description: Memory tracker, 
//		perform allocations through this so that you can keep track of alive memory
class Memory
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Method:    New
	// FullName:  Memory::New
	// Access:    public static 
	// Returns:   constexpr T*
	// Description: Allocate memory of size sizeof(T) * count
	// Parameter: unsigned int count
	template<typename T>
	[[nodiscard]] constexpr static T* New(unsigned int count = 1U)
	{
		const auto size = sizeof(T);
		auto pObj = reinterpret_cast<T*>(calloc(count, size));

		if (pObj)
		{
			m_TotalMemory += size * count;

			// Update pointer pool
			m_PointerPool[reinterpret_cast<void*>(pObj)] = size * count;
			return pObj;
		}

		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Delete
	// FullName:  Memory::Delete
	// Access:    public static 
	// Returns:   constexpr void
	// Description: Deallocate memory at given location, 
	//		provided that it was allocated by the memory tracker
	// Parameter: T* obj
	// Parameter: bool callDestructor
	template<typename T>
	constexpr static void Delete(T* pObj, bool callDestructor = true)
	{
		const auto size = sizeof(T);
		const auto it = m_PointerPool.find(reinterpret_cast<void*>(pObj));

		if (it != m_PointerPool.cend())
		{
			if(callDestructor)
				pObj->~T();

			free(pObj);
			m_TotalMemory -= it->second;
			m_PointerPool.erase(it);
		}
		else
			std::cout << "Memory address not allocated by this memory manager" << std::endl;
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    HasLeaks
	// FullName:  Memory::HasLeaks
	// Access:    public static 
	// Returns:   bool
	// Qualifier: noexcept
	[[nodiscard]] static bool HasLeaks() noexcept
	{
		return !m_PointerPool.empty();
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetMemoryStatus
	// FullName:  Memory::GetMemoryStatus
	// Access:    public static 
	// Returns:   MemoryStatus
	// Qualifier: noexcept
	[[nodiscard]] static MemoryStatus GetMemoryStatus() noexcept
	{
		return { m_TotalMemory, (uint32_t)m_PointerPool.size() };
	}

private:
	static uint32_t m_TotalMemory;

	// Keep track of a sized pointer
	static std::unordered_map<void*, uint32_t> m_PointerPool;
};

#endif // !MEMORY_TRACKER_H