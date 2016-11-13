#pragma once
#include "Widget.h"
class Panel :
	public Widget
{
public:
	Panel(std::string name, rect2D boundary, glm::uvec4 color = glm::uvec4(255,255,255,255), glm::uint refValue = 255);
	virtual ~Panel();

	glm::vec4 getColor() const;

protected:
	glm::uvec4 m_color;
	glm::uint m_colorRef = 255; //Usely it's 255, you do the value/refValue(0/255)
};

