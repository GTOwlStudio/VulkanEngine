#include "Panel.h"



Panel::Panel(std::string wName, rect2D boundary, glm::uvec4 color, glm::uint colorRef) : Widget(wName, boundary), m_color(color), m_colorRef(colorRef)
{
}


Panel::~Panel()
{
}

uint32_t Panel::gSize()
{
	return (meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexC)) + (meshhelper::QUAD_INDICES_COUNT*sizeof(uint32_t));
}

void Panel::gData(void* arr)
{
	VertexC* tmpV = new VertexC[meshhelper::QUAD_VERTICES_COUNT];
	meshhelper::quadVertices(m_boundary.offset.x, m_boundary.offset.y, m_boundary.extent.width, m_boundary.extent.height, m_depth, glm::vec3(getColor()), tmpV);
	memcpy(arr, tmpV, meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexC));
	delete[] tmpV;
}

void Panel::gIndices(void * arr)
{
	uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
	meshhelper::quadIndices(tmpI);
	memcpy(arr,tmpI, meshhelper::QUAD_INDICES_COUNT*sizeof(uint32_t));
	delete[] tmpI; 
}

glm::vec4 Panel::getColor() const
{
	return (glm::vec4)m_color/glm::vec4(static_cast<float>(m_colorRef));
}
