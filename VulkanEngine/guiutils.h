#pragma once

#include <vulkan\vulkan.h>
#include <vector>


namespace guiUtils
{
	void getMiddleCoord(VkRect2D const& parentRect,
		VkExtent2D const& childSize,
		VkOffset2D* dstCoord);

	bool intersect(VkRect2D const& obj, VkOffset2D point);
	
	void generateHorizontalList(VkOffset2D startCoord,VkRect2D* objList, size_t size); 

}