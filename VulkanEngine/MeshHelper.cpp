#include "MeshHelper.h"

namespace meshhelper 
{
	bool isAnEdge(uint32_t a, uint32_t b, std::vector<uint32_t> indicesCollection)
	{
		if (a==b) {
			return false; //There is no need to look
		}
		uint32_t fa, fb; // We want a<b
		fa = std::min(a,b);
		fb = std::max(a,b);
		for (size_t i = 1; i < indicesCollection.size();i++) {
			if (indicesCollection[i]==b&&indicesCollection[i-1]==a) {
				return true;
			}
		}
		return false;
	}

	void quadIndices(uint32_t * dstArr)
	{
		uint32_t tmp[6] = {0,1,2,0,2,3};
		for (uint32_t i = 0; i < 6;i++) {
			dstArr[i] = tmp[i];
		}
	}
	void quadVertices(float x, float y, float w, float h, float depth, glm::vec3 color, VertexC * dstArr)
	{
		VertexC tmp[4] = { VertexC(x,y,depth, color), VertexC(x + w,y,depth,color), VertexC(x+w,y+h,depth, color), VertexC(x,y + h,depth, color) };
		for (uint32_t i = 0; i < 4;i++) {
			dstArr[i] = tmp[i];
		}
	}
}