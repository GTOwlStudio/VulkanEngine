#pragma once

#include "IInput.h"

class CInput : public IInput
{
public:
	CInput();
	~CInput();

	virtual void update(WPARAM wParam, LPARAM lParam);
	virtual bool getKey(uint32_t keyCode);
	virtual bool getMouseButton(uint8_t keyCode);
	virtual float getX();
	virtual float getY();
	virtual float getMouseWheel();

protected:

	void init();

	bool m_keys[256];
	bool m_mouseKeys[8];

	float m_x;
	float m_y;
	float m_mouseWheel;

};

