#include "RessourcesManager.h"



RessourcesManager::RessourcesManager()
{
}


RessourcesManager::~RessourcesManager()
{
	for (CFont* f : m_fonts ) {
		if (f!=nullptr) {
			delete f;
		}
	}
}

CFont * RessourcesManager::getCFont(std::string fontname, uint32_t size)
{
	if (!exist(fontname, size)) {
		addFont(fontname, size);
	}
	return m_fonts.back();
}

void RessourcesManager::addFont(std::string fontname, uint32_t size)
{
	if (exist(fontname,size)) {
		return;
	}
	m_fonts.push_back(new CFont(std::string(gEnv->getAssetpath() + "./data/fonts/" + fontname + ".ttf"), size));
	m_fonts.back()->load();
	std::string fname = m_fonts.back()->getFontName();
	std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);
	fname += m_fonts.back()->getFontSize();
	m_fontid.push_back(fname);
}

bool RessourcesManager::exist(std::string fontname, uint32_t size)
{
	std::string fname = fontname;
	std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);
	fontname += size;
	for (std::string n : m_fontid) {
		if (n==fname) {
			return true;
		}
	}
	return false;
}

void RessourcesManager::prepareShaders()
{
	std::string s_color_name = "color";
	std::vector<VkVertexInputBindingDescription> s_color_b = {vkTools::initializers::vertexInputBindingDescription(0, sizeof(VertexC), VK_VERTEX_INPUT_RATE_VERTEX)};
	std::vector<VkVertexInputAttributeDescription> s_color_a = { vkTools::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, 3*sizeof(float))
	};
	gEnv->pRenderer->addShader(gEnv->getAssetpath() + "shaders/color.vert.spv", 
		gEnv->getAssetpath() + "shaders/color.frag.spv", &s_color_name, std::vector<VkDescriptorSetLayoutBinding>{}, s_color_b, s_color_a);
}
