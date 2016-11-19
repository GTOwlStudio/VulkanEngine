#include "Panel.h"



Panel::Panel(std::string wName, rect2D boundary, glm::uvec4 color, glm::uint colorRef) : Widget(wName, boundary), m_color(color), m_colorRef(colorRef)
{
}


Panel::~Panel()
{
}

glm::vec4 Panel::getColor() const
{
	return (glm::vec4)m_color/glm::vec4(static_cast<float>(m_colorRef));
}
