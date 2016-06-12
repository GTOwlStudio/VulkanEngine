#pragma once

#include "TEngine.h"
#include "Input.h"
#include "TGui.h"
#include "DO.h"


class TGui;
class DO;

class TCore
{
public:
	TCore(bool enableValidation);
	~TCore();
	void init(HINSTANCE hInstance, WNDPROC WndProc);
	void run();
	virtual void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	TEngine* getTEngine() const;
	Input* getInput() const;
	TGui* getTGui() const;
	

protected:
	void update();
	TEngine *m_pEngine;
	Input *m_pInput;
	TGui *m_pGui;
	//DO *d;


private:

	float timer = 0.0f;
	float timerSpeed = 0.25f;
	bool paused = false;
	float frameTimer = 1.0f;
	float fpsTimer = 0.0f;
	uint32_t frameCounter = 0;
	
};

