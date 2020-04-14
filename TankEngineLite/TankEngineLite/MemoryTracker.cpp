#include "MemoryTracker.h"

unsigned int Memory::m_TotalMemory = 0;
std::unordered_map<void*, uint32_t> Memory::m_PointerPool{};