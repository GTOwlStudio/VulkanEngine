#include "System.h"

SSystemGlobalEnvironement* gEnv = NULL;

CSystem::CSystem(bool enableValidation)
{

	memset(&m_env, 0, sizeof(m_env));

	m_env.pSystem = this;
	m_env.enableValidation = enableValidation;
	m_env.pRenderer = new CRenderer();
	m_env.pInput = new CInput();
	gEnv = &m_env;

}
CSystem::~CSystem() 
{
	delete(m_env.pRenderer);
	delete(m_env.pInput);
	 
	gEnv = 0;
}

bool CSystem::Init(HINSTANCE hInstance, WNDPROC wndProc)
{
	m_env.pRenderer->InitVulkan();
	if (m_env.enableValidation) {
		setupConsole("tEngine Console");
	}
	setupWindow(hInstance, wndProc);
	
	m_env.pRenderer->Init();
	return true;
}



void CSystem::renderLoop()
{
#if defined(_WIN32)
	MSG msg;
	while (!m_isFinished) {
		auto tStart = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		
		m_env.pRenderer->render();

		m_frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd-tStart).count();
		frameTimer = (float)tDiff / 1000.0f;
		
		if (!m_paused) {
			m_timer += m_timerSpeed * frameTimer;
			if (m_timer>1.0f) {
				m_timer -= 1.0f;
			}
		}
		m_fpsTimer += (float)tDiff;
		if (m_fpsTimer>1000.0f) {
			std::string windowTitle = getWindowTitle();
			SetWindowText(m_window, windowTitle.c_str());
			m_fpsTimer = 0.0f;
			m_frameCounter = 0.0f;
		}

	}
#endif
}

void CSystem::Update()
{

}

void CSystem::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		ValidateRect(m_window, NULL);
		break;
	case WM_KEYDOWN:
		m_env.pRenderer->handleMessages(wParam, lParam);
		switch (wParam) {
		case VK_ESCAPE:
			m_isFinished = true;
			break;
		}
		
		

		break;
	}
}

std::string CSystem::getWindowTitle() const
{
	return m_sTitle;
}

std::string CSystem::getAppName() const
{
	return m_sName;
}

HWND CSystem::getWindow() const
{
	return m_window;
}

HINSTANCE CSystem::getWindowInstance() const
{
	return m_windowInstance;
}

uint32_t CSystem::getWidth() const
{
	return m_iWidth;
}

uint32_t CSystem::getHeight() const
{
	return m_iHeight;
}

uint32_t * CSystem::getWidthPtr()
{
	return &m_iWidth;
}

uint32_t * CSystem::getHeightPtr()
{
	return &m_iHeight;
}



HWND CSystem::setupWindow(HINSTANCE hInstance, WNDPROC wndProc)
{
	this->m_windowInstance = hInstance;

	bool fullscreen = false;

	// Check command line arguments
	for (int32_t i = 0; i < __argc; i++)
	{
		if (__argv[i] == std::string("-fullscreen"))
		{
			fullscreen = true;
		}
	}

	WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = m_sName.c_str();
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((m_iWidth != screenWidth) && (m_iHeight != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = FALSE;
				}
				else
				{
					return FALSE;
				}
			}
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	if (fullscreen)
	{
		windowRect.left = (long)0;
		windowRect.right = (long)screenWidth;
		windowRect.top = (long)0;
		windowRect.bottom = (long)screenHeight;
	}
	else
	{
		windowRect.left = (long)screenWidth / 2 - m_iWidth / 2;
		windowRect.right = (long)m_iWidth;
		windowRect.top = (long)screenHeight / 2 - m_iHeight / 2;
		windowRect.bottom = (long)m_iHeight;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = getWindowTitle();
	m_window = CreateWindowEx(0,
		m_sName.c_str(),
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		windowRect.left,
		windowRect.top,
		windowRect.right,
		windowRect.bottom,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!m_window)
	{
		printf("Could not create window!\n");
		fflush(stdout);
		return 0;
		exit(1);
	}

	ShowWindow(m_window, SW_SHOW);
	SetForegroundWindow(m_window);
	SetFocus(m_window);

	return m_window;
}

void CSystem::setupConsole(std::string title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle(TEXT(title.c_str()));
	if (m_env.enableValidation)
	{
		std::cout << "Validation enabled:\n";
	}
}
