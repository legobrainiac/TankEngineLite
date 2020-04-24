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

#ifdef _WIN32
#include <Windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif