#pragma once
#include "Widget.h"
#include "Font.h"


class Label : public Widget
{
public:
	Label(std::string text, rect2D boundary, std::string fontname = "segoeui", uint32_t fontSize = 12);
	Label(std::string text, offset2D position, std::string fontname = "segoeui", uint32_t fontSize = 12);
	~Label();
	

protected:
	std::string m_text;
	//f m_f;
	CFont m_font;
};

