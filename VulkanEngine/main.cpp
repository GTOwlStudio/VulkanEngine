#include <stdio.h>
#include <iostream>

#include "TCore.h"

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif
/*
int Run() {
	
}*/

TCore *core;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (core != NULL) {
		core->getTEngine()->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
#endif
{
	int nRes;
//	nRes = Run();
	core = new TCore(true);
	core->init(hInstance, WndProc);
	core->run();

	delete core;
	return 0;

	//return nRes;
}