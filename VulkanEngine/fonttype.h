#pragma once

#include <stdint.h>

struct character_info
{
	float ax; //x advance
	float ay;

	uint32_t bw; //bitmap width
	uint32_t bh;

	float w; //font width
	float h; //font height

	float bx; //X bearing
	float by; //Y bearing

	float tx; //x offset of glyph in texture coordinates
	float ty; //y offset of glyph in texture coordinates

};