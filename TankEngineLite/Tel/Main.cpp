#include "TelPCH.h"

#if _DEBUG
#include <vld.h>
#endif

#include "SDL.h"
#include "Tel.h"

int main(int, char*[]) 
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
    
	TEngine engine;
	engine.Run();
    return 0;
}