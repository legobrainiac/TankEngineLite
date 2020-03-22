#ifndef PCH_H
#define PCH_H

#include <stdio.h>
#include <iostream> // std::cout
#include <sstream> // stringstream
#include <memory> // smart pointers
#include <vector>

#include "Pool.h"

#define INTE
#define ECS_LOG
#include "ecs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // !PCH_H