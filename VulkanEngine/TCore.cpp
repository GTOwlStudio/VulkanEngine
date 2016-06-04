#include "TCore.h"



TCore::TCore(bool enableValidation)
{
	m_pEngine = new TEngine(enableValidation);
}


TCore::~TCore()
{
	delete m_pEngine;
	m_pEngine = 0;
}

void TCore::init(HINSTANCE hInstance, WNDPROC WndProc)
{
	m_pEngine->setupWindow(hInstance, WndProc);
	m_pEngine->initSwapchain();
	m_pEngine->prepare();

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

TEngine* TCore::getTEngine() const
{
	return m_pEngine;
}
