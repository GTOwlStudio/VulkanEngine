#include "guiutils.h"

namespace guiUtils 
{
	void getMiddleCoord(VkRect2D const& parentRect,
		VkExtent2D const& childSize, VkOffset2D* dstCoord) {
	
		dstCoord->x = parentRect.offset.x - (int32_t)(0.5f*(parentRect.extent.width - childSize.width));
		dstCoord->y = parentRect.offset.y - (int32_t)(0.5f*(parentRect.extent.height- childSize.height));
	}

	bool intersect(VkRect2D const& obj, VkOffset2D point) {
		return (point.x >=obj.offset.x)&&(point.x<=(obj.offset.x + (int32_t)obj.extent.width)) &&
			(point.y >= obj.offset.y) && (point.y <= (obj.offset.y + (int32_t)obj.extent.height));
	}

	void generateHorizontalList(VkOffset2D startCoord,VkRect2D* objList, size_t size)
	{
		//First read : get the width of every Object

		objList[0].offset.x = startCoord.x;
		objList[0].offset.y = startCoord.y;

		for (size_t i = 1; i < size;i++) {
			objList[i].offset.x = objList[i - 1].offset.x + objList[i - 1].extent.width;
			objList[i].offset.y = objList[i - 1].offset.y + objList[i - 1].extent.height;
		}

	}

}

