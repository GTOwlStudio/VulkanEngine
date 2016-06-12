#include "guiButton.h"



guiButton::guiButton(float x, float y, float w, float h, float depth, Input &input) : _isEntered(false), _isExited(true),
_depth(depth), _input(input)
{
	rect.offset.x = x;
	rect.offset.y = y;
	rect.extent.width = w;
	rect.extent.height = h;
}


guiButton::~guiButton()
{
}

void guiButton::updateLogic()
{
	if (guiTools::intersection(_input.GetMousePos(), rect)) {

		if (!_isEntered) {
			_isEntered = true;
			printf("Entered\n");
		}
	}
	else {
		if (_isEntered) {
			_isEntered = false;
		}
	}
}

bool guiButton::isEntered()
{
	return _isEntered;
}

bool guiButton::isExited()
{
	return _isExited;
}

const std::vector<GData>& guiButton::getData()
{
#define color {1.0f, 1.0f, 1.0f}
	float x, y, w, h;
	x = rect.offset.x;
	y = rect.offset.y;
	w = rect.extent.width;
	h = rect.extent.height;
	std::vector<GData> vertex =
	{
		{ { x + w,y + h,	_depth },	color },
		{ { x	,y + h,	_depth },	color },
		{ { x	,y,		_depth },	color },
		{ { x + w,y,	_depth },	color }
	};
#undef color
	return vertex;
}

const std::vector<uint32_t>& guiButton::getIndices()
{
	std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
	return indices;
}
