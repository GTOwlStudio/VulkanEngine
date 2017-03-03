#include "guilabel.h"



guilabel::guilabel(std::string text, rect2D boundary, std::string fontname, uint32_t fsize, bool unpack)// : m_text(text)
/*, m_font(*gEnv->pRessourcesManager->getCFont(fontname, fsize))*/: Widget(text,  boundary, text)
{
	m_className = "guilabel";
	m_font = new CFont(*gEnv->pRessourcesManager->getCFont(fontname, fsize));
	
	if (unpack) {
		m_bufferId = 
			gEnv->pMemoryManager->requestMemory((meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexT)*text.size()),
			"Label"+text.substr(0,std::min(text.length(),size_t(4))));
	}
	else {
		m_bufferId =
			gEnv->pMemoryManager->requestMemory(((meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)) + 
				(meshhelper::QUAD_INDICES_COUNT*sizeof(uint32_t)))*text.size(),
				"Label" + text.substr(0, std::min(text.length(), size_t(4))));
	}
}

guilabel::guilabel(std::string text, offset2D position, std::string fontname, uint32_t fsize, bool unpack) : 
	//guilabel(text, rect2D(position, guitools::getTextSize(text, *gEnv->pRessourcesManager->getCFont(fontname,fsize))))
	guilabel(text, rect2D(position,guitools::getTextSize(text,"./data/fonts/"+fontname+".ttf",fsize)), fontname, fsize, unpack)
{

}

guilabel::~guilabel()
{
}

VkDeviceSize guilabel::gSize()
{
	return ((meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)) +
		(meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)))*m_text.size();
}

VkDeviceSize guilabel::gDataSize()
{
	return meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)*m_text.size();
}

VkDeviceSize guilabel::gIndicesSize()
{
	return meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)*m_text.size();
}

void guilabel::gData(void * arr)
{
	VertexT* tmpV = new VertexT[meshhelper::QUAD_VERTICES_COUNT*m_text.size()];
	character_info ci[256];
	m_font->getCharacterInfo(ci);
	glm::vec3 pos = glm::vec3(m_boundary.offset.x, m_boundary.offset.y,m_depth);
	for (size_t i = 0; i < m_text.size();i++) {
		tmpV[4*i].pos = glm::vec3(
			ci[int(i)].bx,
			ci[int(i)].by,
			m_depth) + pos;
		tmpV[4 * i + 1].pos = glm::vec3(
			ci[int(i)].bx+ci[int(i)].w,
			ci[int(i)].by,
			m_depth)+pos;
		tmpV[4 * i + 2].pos = glm::vec3(
			ci[int(i)].bx + ci[int(i)].w,
			ci[int(i)].h-ci[int(i)].by,
			m_depth)+pos;
		tmpV[4 * i + 3].pos = glm::vec3(
			ci[int(i)].bx,
			ci[int(i)].h - ci[int(i)].by, 
			m_depth)+pos;
		pos += glm::vec3(ci[int(i)].bx + ci[int(i)].w, 0, 0);
		tmpV[4 * i].tc = glm::vec2(ci[int(i)].tx, ci[int(i)].ty+ci[int(i)].bh);
		tmpV[4 * i +1].tc = glm::vec2(ci[int(i)].tx+ci[int(i)].bw, ci[int(i)].ty + ci[int(i)].bh);
		tmpV[4 * i + 2].tc = glm::vec2(ci[int(i)].tx+ci[int(i)].bw, ci[int(i)].ty);
		tmpV[4 * i + 3].tc = glm::vec2(ci[int(i)].tx, ci[int(i)].ty);
	}
	memcpy(arr,tmpV,meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexT)*m_text.size());
	delete[] tmpV;
}

void guilabel::gIndices(void * arr)
{
	uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT*m_text.size()];
	uint32_t* tmpPattern = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
	meshhelper::quadIndices(tmpPattern);
	
	for (size_t i = 0; i < m_text.size();i++) {
		for (size_t j = 0; j < meshhelper::QUAD_INDICES_COUNT;j++) {
			tmpI[i*meshhelper::QUAD_INDICES_COUNT + j] = meshhelper::QUAD_VERTICES_COUNT*i + tmpPattern[j];
		}
	}
	memcpy(arr, tmpI, meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)*m_text.size());
	delete[] tmpPattern;
	delete[] tmpI;
}
