#pragma once

#include "Widget.h"
#include "Font.h"
#include "guitools.h"

class CFont;

class guilabel : public Widget
{
public:
	guilabel(std::string text, rect2D boundary, std::string fontname = "segoeui", uint32_t fontSize = 12, bool unpack=false);
	guilabel(std::string text, offset2D position, std::string fontname = "segoeui", uint32_t fontSize = 12, bool unpack=false);
	virtual ~guilabel();
	
	virtual VkDeviceSize gSize();
	virtual VkDeviceSize gDataSize();
	virtual VkDeviceSize gIndicesSize();
	virtual void gData(void* arr);
	virtual void gIndices(void* arr);

	//glm::vec4 getColor();

protected:

	//std::string m_text;
	//f m_f;
	CFont* m_font;
};

