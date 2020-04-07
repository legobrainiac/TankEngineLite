#include "pch.h"

#if _DEBUG
#include <vld.h>
#endif

#include "Tel.h"
#include "MainGame.h"

int main(int, char* [])
{
	TEngineRunner engine;
	engine.Run<MainGame>();
	return 0;
}