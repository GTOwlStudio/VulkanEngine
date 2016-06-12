#include "Input.h"

Input::Input() : mousePos(), escape(false)
{

	resetKey(); //initialize the key to false

}

Input::~Input() {

}

void Input::updateKeys(WPARAM wParam)
{
	switch (wParam) {
	case eK_ESCAPE: {
			keys[eKI_ESCAPE] = true;
			break;
	}
	case eK_G: {
		keys[eKI_G] = true;
		break;
	}
	}
}

void Input::updateMouse(LPARAM lParam)
{
	int32_t posx = LOWORD(lParam);
	int32_t posy = HIWORD(lParam);
	mousePos = glm::vec2((float)posx, (float)posy);
}

bool Input::getKey(EKeyId keyCode)
{
	if (keys[keyCode]) {
		return true;
	}
	return false;
}

glm::vec2 Input::GetMousePos() const
{
	return mousePos;
}

void Input::resetKey()
{
	for (size_t i = 0; i < EKEY_NUM; i++) {
		keys[i] = false;
	}
}
