#pragma once

//#include "Widget.h"
#include "guitype.h"
#include "Font.h"
#include "nongraphicfont.h"
//#include "Widget.h"

#include <vulkan\vulkan.h>
#include <vector>
#include <glm\glm.hpp>
#include "vulkanTools\Helper.h"

class CFont;

namespace guitools
{
	void getMiddleCoord(rect2D const& parentRect,
		extent2D const& childSize,
		offset2D* dstCoord);

	bool intersect(rect2D const& obj, offset2D point);
	glm::vec3 hexaColor(std::string hexaCode);
	
	void generateHorizontalList(offset2D startCoord,rect2D* objList, size_t size); 

	offset2D center(rect2D parent, extent2D child);
	extent2D getTextSize(std::string text, CFont& font);
	extent2D getTextSize(std::string text, std::string fontName, uint32_t fontSize);
	bool entered(rect2D surface, offset2D xy);

}