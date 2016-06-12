#pragma once
#include "ITGuiObject.h"
#include "Input.h"

class guiButton: public ITGuiObject
{
public:
	guiButton(float x, float y, float w, float h, float depth, Input &input);
	virtual ~guiButton();
	
	virtual void updateLogic();

	virtual bool isEntered();
	virtual bool isExited();

	virtual const std::vector<GData> &getData();
	virtual const std::vector<uint32_t> &getIndices();
	


protected:

	Input &_input;

	bool _isExited;
	bool _isEntered;

	VkRect2D rect;
	float _depth;

};

