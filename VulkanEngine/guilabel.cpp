#include "guilabel.h"



guilabel::guilabel(std::string text, rect2D boundary, std::string fontname, uint32_t fsize, bool genIdx, bool genVtx)// : m_text(text)
/*, m_font(*gEnv->pRessourcesManager->getCFont(fontname, fsize))*/: Widget(text,  boundary, text)
{
	m_className = "guilabel";
	//m_font = new CFont(*gEnv->pRessourcesManager->getCFont(fontname, fsize));
	m_font = gEnv->pRessourcesManager->getCFont(fontname, fsize);

	if (genVtx && !genIdx) {
		//printf("%i\n", (meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)*text.size()));
		m_bufferId = 
			gEnv->pMemoryManager->requestMemory((meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexT)*(text.size() - helper::countCharacter(m_text, ' '))),
			"Label"+text.substr(0,std::min(text.length(),size_t(4))));
	}
	else if (genVtx && genIdx) {
		m_bufferId =
			gEnv->pMemoryManager->requestMemory(((meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)) + 
				(meshhelper::QUAD_INDICES_COUNT*sizeof(uint32_t)))*(text.size() - helper::countCharacter(m_text, ' ')),
				"Label" + text.substr(0, std::min(text.length(), size_t(4))));
	}
}

guilabel::guilabel(std::string text, offset2D position, std::string fontname, uint32_t fsize, bool genIdx, bool genVtx) : 
	//guilabel(text, rect2D(position, guitools::getTextSize(text, *gEnv->pRessourcesManager->getCFont(fontname,fsize))))
	guilabel(text, rect2D(position,guitools::getTextSize(text,"./data/fonts/"+fontname+".ttf",fsize)), fontname, fsize, genIdx, genVtx)
{

}

guilabel::~guilabel()
{
}

VkDeviceSize guilabel::gSize()
{
	return ((meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)) +
		(meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)))*(m_text.size()-helper::countCharacter(m_text, ' '));
}

VkDeviceSize guilabel::gDataSize()
{
	return meshhelper::QUAD_VERTICES_COUNT * sizeof(VertexT)*(m_text.size() - helper::countCharacter(m_text, ' '));
}

VkDeviceSize guilabel::gIndicesSize()
{
	return meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)*(m_text.size() - helper::countCharacter(m_text, ' '));
}

void guilabel::gData(void * arr)
{
	printf("%i\n", helper::countCharacter(m_text, ' '));
	VertexT* tmpV = new VertexT[meshhelper::QUAD_VERTICES_COUNT*(m_text.size()-helper::countCharacter(m_text, ' '))]; //delete space
	character_info ci[256];
	glm::vec2 screen_size = glm::vec2((float)m_font->getAtlasWidth(), (float)m_font->getAtlasHeight());
	float s_width = screen_size.x;
	float s_height = screen_size.y;
	m_font->getCharacterInfo(ci);

	glm::vec3 pos = glm::vec3(m_boundary.offset.x, m_boundary.offset.y+m_boundary.extent.height,m_depth);
	int charId;
	int proofreaderId = 0; //Blank space are not considered as 
	int i = 0;
	for (size_t id = 0; id < m_text.size();id++) {
		//printf("%c", (int)m_text[i]);
		i = id - proofreaderId;
		charId = int(m_text[id]);
		/*tmpV[4*i].pos = glm::vec3(
			ci[charId].bx,
			ci[charId].by,
			m_depth) + pos;
		tmpV[4 * i + 1].pos = glm::vec3(
			ci[charId].bx+ci[charId].w,
			ci[charId].by,
			m_depth)+pos;
		tmpV[4 * i + 2].pos = glm::vec3(
			ci[charId].bx + ci[charId].w,
			ci[charId].h-ci[charId].by,
			m_depth)+pos;
		tmpV[4 * i + 3].pos = glm::vec3(
			ci[charId].bx,
			ci[charId].h - ci[charId].by, 
			m_depth)+pos;*/
		if (charId == 32) {
			pos += glm::vec3(m_font->getFontSize()/4, 0, 0);
			proofreaderId += 1;
			continue;
		}
		tmpV[4 * i].pos = glm::vec3(
			ci[charId].bx,
			-ci[charId].by,
			m_depth) + pos;
		tmpV[4 * i + 1].pos = glm::vec3(
			ci[charId].bx + ci[charId].w,
			-ci[charId].by,
			m_depth) + pos;
		tmpV[4 * i + 2].pos = glm::vec3(
			ci[charId].bx + ci[charId].w,
			-ci[charId].by + ci[charId].h,
			m_depth) + pos;
		tmpV[4 * i +3].pos = glm::vec3(
			ci[charId].bx,
			-ci[charId].by + ci[charId].h,
			m_depth) + pos;
		pos += glm::vec3(ci[charId].bx + ci[charId].w, 0, 0);

		tmpV[4 * i +3].tc = glm::vec2(ci[charId].tx, ci[charId].ty+(ci[charId].bh/s_height));
		tmpV[4 * i +2].tc = glm::vec2(ci[charId].tx+(ci[charId].bw/s_width), ci[charId].ty + (ci[charId].bh/s_height));
		tmpV[4 * i + 1].tc = glm::vec2(ci[charId].tx+(ci[charId].bw/s_width), ci[charId].ty);
		tmpV[4 * i].tc = glm::vec2(ci[charId].tx, ci[charId].ty);



	}
	memcpy(arr,tmpV,meshhelper::QUAD_VERTICES_COUNT*sizeof(VertexT)*(m_text.size()-helper::countCharacter(m_text, ' ')));
	delete[] tmpV;
}

void guilabel::gIndices(void * arr)
{

	uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT*m_text.size()- helper::countCharacter(m_text, ' ')];
	uint32_t* tmpPattern = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
	meshhelper::quadIndices(tmpPattern);
	
	for (size_t i = 0; i < m_text.size()- helper::countCharacter(m_text, ' ');i++) {
		for (size_t j = 0; j < meshhelper::QUAD_INDICES_COUNT;j++) {
			tmpI[i*meshhelper::QUAD_INDICES_COUNT + j] = meshhelper::QUAD_VERTICES_COUNT*i + tmpPattern[j];
		}
	}
	memcpy(arr, tmpI, meshhelper::QUAD_INDICES_COUNT * sizeof(uint32_t)*(m_text.size()- helper::countCharacter(m_text, ' ')));
	delete[] tmpPattern;
	delete[] tmpI;
}
