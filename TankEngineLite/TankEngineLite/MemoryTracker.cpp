#include "MemoryTracker.h"

unsigned int Memory::m_TotalMemory = 0;
std::map<void*, uint32_t> Memory::m_PointerPool{};