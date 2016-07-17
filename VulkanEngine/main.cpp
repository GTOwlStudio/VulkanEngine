
#include <iostream>

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "System.h"

//extern SSystemGlobalEnvironement *gEnv = NULL;

CSystem *sys;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	if (sys != NULL)
	{
		sys->handleMessages(hWnd, uMsg, wParam, lParam);
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	sys = new CSystem(true);
	sys->Init(hInstance, WndProc);
	sys->renderLoop();
	system("pause");
	delete sys;
	return 0;
}