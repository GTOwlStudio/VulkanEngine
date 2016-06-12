#include "TCore.h"



TCore::TCore(bool enableValidation)
{
	m_pEngine = new TEngine(enableValidation);
	m_pInput = new Input();
	m_pGui = new TGui(*this, *m_pInput);
	//d = new DO(*this);
}


TCore::~TCore()
{
	delete m_pGui;
	delete m_pInput;
	delete m_pEngine;
	//delete d;
	//d = 0;
	m_pEngine = 0;
	m_pInput = 0;
	m_pGui = 0;
}

void TCore::init(HINSTANCE hInstance, WNDPROC WndProc)
{
	m_pEngine->setupWindow(hInstance, WndProc);
	m_pEngine->initSwapchain();
	m_pEngine->prepare();

	//m_pGui->load();
}

void TCore::run()
{
#if defined(_WIN32)
	MSG msg;
	while (TRUE) {
		auto tStart = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		m_pGui->updateLogic();
		m_pEngine->render();

		frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		frameTimer = (float)tDiff / 1000.0f;

		if (!paused) {
			timer += timerSpeed * frameTimer;
			if (timer>1.0f) {
				timer -= 1.0f;
			}
		}
		fpsTimer += (float)tDiff;
		if (fpsTimer >1000.0f) {
			std::string windowTitle = m_pEngine->getWindowTitle();
			SetWindowText(m_pEngine->window, windowTitle.c_str());
			fpsTimer = 0.0f;
			frameCounter = 0.0f;
		}

	}
#endif
}

void TCore::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pInput->updateKeys(wParam);
	m_pInput->updateMouse(lParam);
	
	if (m_pInput->getKey(eKI_G)) { exit(0); }
}

TEngine* TCore::getTEngine() const
{
	return m_pEngine;
}

Input * TCore::getInput() const
{
	return m_pInput;
}

TGui * TCore::getTGui() const
{
	return m_pGui;
}
