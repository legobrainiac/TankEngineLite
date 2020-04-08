 #ifndef PCH_H
#define PCH_H

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include "ecs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define LOGINFO(MESSAGE)do{ \
std::stringstream stream{};\
stream << MESSAGE;\
OutputDebugString(stream.str().c_str());\
std::cout << stream.str();\
}while(0)\

#define LOGINFOW(MESSAGE)do{ \
std::wstringstream stream{};\
stream << MESSAGE;\
OutputDebugStringW(stream.str().c_str());\
std::wcout << stream.str();\
}while(0)\

#endif // !PCH_H