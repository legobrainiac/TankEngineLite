#include "pch.h"

#include "Tel.h"

#include "MainGame.h"
#include "CTCPUGame.h"
#include "EditorGame.h"

#include <vld.h>
#include <D3D.h>

int main(int, char* [])
{
	//return TEngineRunner{}.Run<CTCPUGame>();
	return TEngineRunner{}.Run<EditorGame>();
	//return TEngineRunner{}.Run<MainGame>();
}

#ifdef _WIN32
#include <Windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif