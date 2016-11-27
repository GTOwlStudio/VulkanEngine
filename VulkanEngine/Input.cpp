#include "Input.h"



CInput::CInput()
{
}


CInput::~CInput()
{
}

void CInput::update(UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

	case WM_KEYDOWN:
		for (uint32_t i = 0; i < 256; i++) {
			m_keys[i] = (wParam == i);
			/*if (wParam == i) {
				//printf("%i %c\n", i, i);
			}*/
		}
		break;

	case WM_KEYUP:
		for (uint32_t i = 0; i < 256; i++) {
			m_keys[i] = (wParam == i);
			/*if (wParam == i) {
				//printf("%i %c\n", i, i);
			}*/
		}
		break;

	case WM_LBUTTONDOWN:
		m_mouseKeys[0] = true;
		break;

	case WM_LBUTTONUP:
		m_mouseKeys[0] = false;
		break;

	case WM_RBUTTONDOWN:
		m_mouseKeys[1] = true;
		break;

	case WM_RBUTTONUP:
		m_mouseKeys[1] = false;
		break;

	case WM_MOUSEMOVE:
		m_x = (float)LOWORD(lParam);
		m_y = (float)HIWORD(lParam);
		break;
	}
}

bool CInput::getKey(uint32_t keyCode)
{
	return false;
}

bool CInput::getMouseButton(uint8_t keyCode)
{
	return false;
}

float CInput::getX()
{
	return m_x;
}

float CInput::getY()
{
	return m_y;
}

float CInput::getMouseWheel()
{
	return 0.0f;
}
