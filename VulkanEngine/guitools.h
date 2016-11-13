#pragma once

#include "Font.h"

#include <vulkan\vulkan.h>
#include <vector>
//#include "Font.h"


struct offset2D {
	float x, y;
	offset2D() : x(0.0f), y(0.0f) {}
	offset2D(float px, float py) : x(px), y(py) {}
};
struct extent2D {
	float width, height;
	extent2D() : width(0.0f), height(0.0f) {}
	extent2D(float w, float h) : width(w), height(h) {}

};

struct rect2D {
	offset2D offset;
	extent2D extent;
	rect2D(offset2D poffset, extent2D pextent) : offset(poffset), extent(pextent) {}
	rect2D(float x, float y, float w, float h) : offset(x, y), extent(w, h) {}
};

namespace guitools
{
	void getMiddleCoord(rect2D const& parentRect,
		extent2D const& childSize,
		offset2D* dstCoord);

	bool intersect(rect2D const& obj, offset2D point);
	
	
	//void generateHorizontalList(extent2D startCoord,rect2D* objList, size_t size); 

	offset2D center(rect2D parent, extent2D child);
	extent2D getTextSize(std::string text, CFont font);
	bool entered(rect2D surface, offset2D xy);

}