#ifndef PCH_H
#define PCH_H

#include <stdio.h>
#include <stdlib.h> 
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include "ecs.h"
#include "Logger.h"
#include "Profiler.h"
#include "MemoryTracker.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define LOGGER Logger::GetInstance()

#endif // !PCH_H