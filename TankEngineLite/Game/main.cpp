#include <vld.h>

#include "pch.h"
#include "Tel.h"

#include "MainGame.h"
#include "CTCPUGame.h"
#include "EditorGame.h"

#include <D3D.h>

int main(int, char* [])
{
	//int result = TEngineRunner{}.Run<CTCPUGame>();
	//int result = TEngineRunner{}.Run<EditorGame>();
	int result = TEngineRunner{}.Run<MainGame>();
	return result;
}

#ifdef _WIN32
#include <Windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif