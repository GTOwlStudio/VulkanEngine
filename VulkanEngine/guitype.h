#pragma once

#include <vector>
#include <string>

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