#include "pch.h"

#if _DEBUG
#include <vld.h>
#endif

#include "Tel.h"
#include "MainGame.h"

#include <D3D.h>

int main(int, char* [])
{
	TEngineRunner engine;
	engine.Run<MainGame>();
	return 0;
}