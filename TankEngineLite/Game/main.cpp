#include "TelPCH.h"

// #if _DEBUG
// #include <vld.h>
// #endif

#define ALLOC_CONSOLE

#include "SDL.h"
#include "Tel.h"

#include "MainGame.h"

int main(int, char* [])
{
#ifdef ALLOC_CONSOLE
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
#endif

	TEngineRunner engine;
	engine.Run<MainGame>();
	return 0;
}