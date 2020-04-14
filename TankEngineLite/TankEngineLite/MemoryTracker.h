#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

struct MemoryStatus
{
	uint32_t m_TotalMemoryCount;
	uint32_t m_MemoryChunckCount;
};

class Memory
{
public:
	template<typename T>
	inline static T* New(unsigned int count = 1U)
	{
		const auto size = sizeof(T);
		T* obj = reinterpret_cast<T*>(calloc(count, size));

		if (obj)
		{
			m_TotalMemory += size * count;

			// Update pointer pool
			m_PointerPool[reinterpret_cast<void*>(obj)] = size * count;
			return obj;
		}

		return nullptr;
	}

	template<typename T>
	inline static void Delete(T* obj, bool callDestructor = true)
	{
		const auto size = sizeof(T);
		auto it = m_PointerPool.find(reinterpret_cast<void*>(obj));

		if (it != m_PointerPool.end())
		{
			if(callDestructor)
				obj->~T();

			free(obj);
			m_TotalMemory -= it->second;
			m_PointerPool.erase(it);
		}
		else
			std::cout << "Memory address no allocated by this memory manager" << std::endl;
	}

	static inline bool HasLeaks()
	{
		return !m_PointerPool.empty();
	}

	static inline MemoryStatus GetMemoryStatus()
	{
		return { m_TotalMemory, (uint32_t)m_PointerPool.size() };
	}

private:
	static uint32_t m_TotalMemory;

	// Keep track of a sized pointer
	static std::map<void*, uint32_t> m_PointerPool;
};

#endif // !MEMORY_TRACKER_H