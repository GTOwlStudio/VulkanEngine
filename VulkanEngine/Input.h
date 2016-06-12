#pragma once

#include <Windows.h>

#include <glm\glm.hpp>

#define EKEY_NUM 37

//class TCore;

enum EKeyId {
	eKI_ESCAPE = 0,
	eKI_0 = 1,
	eKI_1 = 2,
	eKI_2 = 3,
	eKI_3 = 4,
	eKI_4 = 5,
	eKI_5 = 6,
	eKI_6 = 7,
	eKI_7 = 8,
	eKI_8 = 9,
	eKI_9 = 10,
	eKI_A = 11,
	eKI_B = 12,
	eKI_C = 13,
	eKI_D = 14,
	eKI_E = 15,
	eKI_F = 16,
	eKI_G = 17,
	eKI_H = 18,
	eKI_I = 19,
	eKI_J = 20,
	eKI_K = 21,
	eKI_L = 22,
	eKI_M = 23,
	eKI_N = 24,
	eKI_O = 25,
	eKI_P = 26,
	eKI_Q = 27,
	eKI_R = 28,
	eKI_S = 29,
	eKI_T = 30,
	eKI_U = 31,
	eKI_V = 32,
	eKI_W = 33,
	eKI_X = 34,
	eKI_Y = 35,
	eKI_Z = 36,
};

class Input
{
public:
	Input();
	~Input();
	void updateKeys(WPARAM wParam);
	void updateMouse(LPARAM lParam);
	bool getKey(EKeyId keyCode);
	//bool getMouseKey(uint8_t mouseKey);
	glm::vec2 GetMousePos() const;
	bool escape;

protected:
	glm::vec2 mousePos;
	//bool mouseButton[3];
	bool keys[EKEY_NUM];

	void resetKey();
	enum EKeyCode
	{
		eK_ESCAPE = VK_ESCAPE,
		eK_0 = 0x30,
		eK_1 = 0x31,
		eK_2 = 0x32,
		eK_3 = 0x33,
		eK_4 = 0x34,
		eK_5 = 0x35,
		eK_6 = 0x36,
		eK_7 = 0x37,
		eK_8 = 0x38,
		eK_9 = 0x39,
		eK_A = 0x41,
		eK_B = 0x42,
		eK_C = 0x43,
		eK_D = 0x44,
		eK_E = 0x45,
		eK_F = 0x46,
		eK_G = 0x47,
		eK_H = 0x48,
		eK_I = 0x49,
		eK_J = 0x4A,
		eK_K = 0x4B,
		eK_L = 0x4C,
		eK_M = 0x4D,
		eK_N = 0x4E,
		eK_O = 0x4F,
		eK_P = 0x50,
		eK_Q = 0x51,
		eK_R = 0x52,
		eK_S = 0x53,
		eK_T = 0x54,
		eK_U = 0x55,
		eK_V = 0x56,
		eK_W = 0x57,
		eK_X = 0x58,
		eK_Y = 0x59,
		eK_Z = 0x5A,
	};

};

