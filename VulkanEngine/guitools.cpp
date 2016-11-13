#include "guitools.h"

namespace guitools 
{
	void getMiddleCoord(rect2D const& parentRect,
		extent2D const& childSize, offset2D* dstCoord) {
	
		dstCoord->x = parentRect.offset.x - (int32_t)(0.5f*(parentRect.extent.width - childSize.width));
		dstCoord->y = parentRect.offset.y - (int32_t)(0.5f*(parentRect.extent.height- childSize.height));
	}

	bool intersect(rect2D const& obj, offset2D point) {
		return (point.x >=obj.offset.x)&&(point.x<=(obj.offset.x + obj.extent.width)) &&
			(point.y >= obj.offset.y) && (point.y <= (obj.offset.y + obj.extent.height));
	}

	/*void generateHorizontalList(offset2D startCoord,rect2D* objList, size_t size)
	{
		//First read : get the width of every Object

		objList[0].offset.x = startCoord.x;
		objList[0].offset.y = startCoord.y;

		for (size_t i = 1; i < size;i++) {
			objList[i].offset.x = objList[i - 1].offset.x + objList[i - 1].extent.width;
			objList[i].offset.y = objList[i - 1].offset.y + objList[i - 1].extent.height;
		}

	}*/

	offset2D center(rect2D parent, extent2D child)
	{
		offset2D centeredPos = {};
		centeredPos.x = parent.offset.x + (parent.extent.width / 2.0f) - (child.width / 2.0f);
		centeredPos.y = parent.offset.y + (parent.extent.height / 2.0f) - (child.height / 2.0f);
		return centeredPos;
	}

	extent2D getTextSize(std::string text, CFont font)
	{
		float height = 0.0f;
		float width = 0.0f;
		character_info cinfo[256];
		font.getCharacterInfo(cinfo);
		for (size_t i = 0; i < text.size(); i++) {
			if (cinfo[(int)text[i]].h>height) {
				height = cinfo[(int)text[i]].h;
			}
			width += cinfo[int(text[i])].w;
		}
		return extent2D(width, height);
	}

	bool entered(rect2D surface, offset2D xy)
	{
		if ((xy.x >= surface.offset.x) && (xy.x <= (surface.offset.x + surface.extent.width)) &&
			(xy.y >= surface.offset.y) && (xy.y <= (surface.offset.y + surface.extent.height))) {
			return true;
		}
		return false;
	}

}

