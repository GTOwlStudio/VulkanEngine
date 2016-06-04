#pragma once

#include "TEngine.h"

class TCore
{
public:
	TCore(bool enableValidation);
	~TCore();
	void init(HINSTANCE hInstance, WNDPROC WndProc);
	void run();
	TEngine* getTEngine() const;

protected:
	void update();
	TEngine *m_pEngine;


private:

	float timer = 0.0f;
	float timerSpeed = 0.25f;
	bool paused = false;
	float frameTimer = 1.0f;
	float fpsTimer = 0.0f;
	uint32_t frameCounter = 0;
	
};

