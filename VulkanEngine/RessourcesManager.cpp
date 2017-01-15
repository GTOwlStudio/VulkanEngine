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

	std::string s_texture_name = "texture";
	std::vector<VkVertexInputBindingDescription> s_texture_b = { vkTools::initializers::vertexInputBindingDescription(0, sizeof(VertexT), VK_VERTEX_INPUT_RATE_VERTEX) };
	std::vector<VkVertexInputAttributeDescription> s_texture_a = { vkTools::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, 3*sizeof(float))
	};
	std::vector<VkDescriptorSetLayoutBinding> s_texture_dslb = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};
	gEnv->pRenderer->addShader(gEnv->getAssetpath() + "shaders/texture.vert.spv", gEnv->getAssetpath() + "shaders/texture.frag.spv", &s_texture_name, s_texture_dslb, s_texture_b, s_texture_a);
	
	uint32_t s_color_binding_id = 0; 

	std::string s_color_name = "color";
	printf("szieof(glm::vec3) = %i\n3*sizeof(float) = %i\n", sizeof(glm::vec3), sizeof(float)*3);
	std::vector<VkVertexInputBindingDescription> s_color_b = {vkTools::initializers::vertexInputBindingDescription(s_color_binding_id, sizeof(VertexC), VK_VERTEX_INPUT_RATE_VERTEX)};
	std::vector<VkVertexInputAttributeDescription> s_color_a = { vkTools::initializers::vertexInputAttributeDescription(s_color_binding_id, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(s_color_binding_id, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec3))
	};
	std::vector<VkDescriptorSetLayoutBinding> s_color_dslb = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, s_color_binding_id),
	};
	gEnv->pRenderer->addShader(gEnv->getAssetpath() + "shaders/color.vert.spv", 
		gEnv->getAssetpath() + "shaders/color.frag.spv", &s_color_name, s_color_dslb, s_color_b, s_color_a);
}
