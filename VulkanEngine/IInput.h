#pragma once

#include <stdint.h>
#include <Windows.h>

class IInput
{
public:
	virtual ~IInput() {};
	virtual void update(WPARAM wParam, LPARAM lParam) = 0;
	virtual bool getKey(uint32_t keyCode) = 0;
	virtual bool getMouseButton(uint8_t keyCode) = 0;
	virtual float getX() = 0;
	virtual float getY() = 0;
	virtual float getMouseWheel() = 0;
};