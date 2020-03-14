#include "TelPCH.h"

#if _DEBUG
#include <vld.h>
#endif

#include "SDL.h"
#include "Tel.h"

int main(int, char*[]) 
{
	TEngine engine;
	engine.Run();
    return 0;
}