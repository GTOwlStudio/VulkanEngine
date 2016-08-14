#pragma once

#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "Renderer.h"
#include "Input.h"
#include "Font.h"

class CSystem;
class CRenderer;
class IInput;
struct IRenderer;
struct I3DEngine;
class CFont;


struct SSystemGlobalEnvironement
{
	I3DEngine* p3DEngine;
	IInput* pInput;
	CSystem* pSystem;
	IRenderer* pRenderer;
	bool enableValidation; //if you want to debug or not. Must be false if release
};

extern SSystemGlobalEnvironement* gEnv;

class CSystem {
public:
	CSystem(bool enableValidation);
	virtual ~CSystem();

	bool Init(HINSTANCE hInstance, WNDPROC wndProc);

#if defined(_WIN32)
	HWND setupWindow(HINSTANCE hInstance, WNDPROC wndProc);
	void setupConsole(std::string title);
	
#endif

	//virtual void Release();
	virtual SSystemGlobalEnvironement* getGlobalEnvironment() { return &m_env; };

	void renderLoop();

	
	void Update();

	//void GetUsedMemory();

	/*virtual void Quit() = 0;

	virtual bool IsQuitting() = 0;

	virtual void FatalError(const char* sFormat, ...) = 0;*/
	
	virtual void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/*virtual I3DEngine* getI3DEngine() = 0;

	virtual IRenderer* getIRenderer() = 0;
	virtual IInput* getIInput() = 0;*/

	SSystemGlobalEnvironement m_env;

	std::string getWindowTitle() const;

public:
	std::string getAppName() const;
	HWND getWindow() const;
	HINSTANCE getWindowInstance() const;
	uint32_t getWidth() const;
	uint32_t getHeight() const;
	uint32_t* getWidthPtr();
	uint32_t* getHeightPtr();
protected:
	HWND m_window;
	HINSTANCE m_windowInstance;
private:
	
	bool m_paused = false;
	bool m_isFinished = false;

	std::string m_sTitle = "tEngine by TEMA";
	std::string m_sName = "tEngine";

	uint32_t m_iWidth = 1280;
	uint32_t m_iHeight = 720;

	
	float m_timer = 0.0f;
	float m_timerSpeed = 0.025f;

	uint32_t m_frameCounter = 0;
	uint32_t m_cap = 60;
	uint32_t m_frameTime_millisecond;
	float frameTimer = 1.0f;
	float m_fpsTimer = 0.0f;


};



inline CSystem* GetISystem() {
	return gEnv->pSystem;
}