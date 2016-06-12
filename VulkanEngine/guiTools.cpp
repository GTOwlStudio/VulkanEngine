#include "guiTools.h"

bool guiTools::intersection(VkOffset2D a, VkRect2D b)
{
	return ((a.x <= (b.offset.x + b.extent.width)) &&	(a.y<=(b.offset.y+b.extent.height)) && (a.x>=b.offset.x) && (a.y>=b.offset.y) );
}

bool guiTools::intersection(glm::vec2 a, VkRect2D b)
{
	return ((a.x <= (b.offset.x + b.extent.width)) && (a.y <= (b.offset.y + b.extent.height)) && (a.x >= b.offset.x) && (a.y >= b.offset.y));
}
