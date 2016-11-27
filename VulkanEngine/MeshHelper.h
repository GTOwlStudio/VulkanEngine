#pragma once

#include <vector>
#include <algorithm>
#include <glm\glm.hpp>


struct VertexC{
	glm::vec4 pos;
	glm::vec4 color;
	VertexC() {}
	VertexC(float x, float y, float z, float r, float g, float b, float a = 1.0f) : pos(x, y, z, 1.0f), color(r,g,b,a) {}
	VertexC(float x, float y, float z, glm::vec3 pColor) : pos(x, y, z, 1.0f), color(pColor, 1.0f) {};
	VertexC(float x, float y, float z, glm::vec4 pColor) : pos(x, y, z, 1.0f), color(pColor) {};
};

namespace meshhelper {

	const uint32_t QUAD_INDICES_COUNT = 6; //The size of an array containing the indices of a quad
	const uint32_t QUAD_VERTICES_COUNT = 4; //The size of an array containing the vertices of a quad, you need to multiply this number by the vertice type used to get the size in byte

	bool isAnEdge(uint32_t a, uint32_t b, std::vector<uint32_t> indicesCollection);

	void quadIndices(uint32_t* dstArr);
	void quadVertices(float x, float y, float w, float h, float depth, glm::vec3 color, VertexC* dstArr);

}
