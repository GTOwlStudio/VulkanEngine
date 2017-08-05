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
	m_fonts.push_back(new CFont(std::string(gEnv->getAssetpath() + "fonts/" + fontname + ".ttf"), size));
	if (m_loaded) {
		m_fonts.back()->load();
	}
	else {
		m_fontsToLoad.push_back(fontname);
		m_fontsToLoadSize.push_back(size);
	}
	//std::string fname = m_fonts.back()->getFontName();
	std::string fname = fontname;
	std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);
	fname += std::to_string(m_fonts.back()->getFontSize());
	m_fontid.push_back(fname);
}

bool RessourcesManager::exist(std::string fontname, uint32_t size)
{
	std::string fname = fontname;
	std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);
	fname += std::to_string(size);
	for (std::string n : m_fontid) {
		printf("%s %s\n", n.c_str(), fname.c_str());
		if (n==fname) {
			return true;
		}
	}
	return false;
}

void RessourcesManager::finishLoading()
{
	for (size_t i = 0; i < m_fontsToLoad.size();i++) {
		getCFont(m_fontsToLoad[i],m_fontsToLoadSize[i])->load();
	}
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
	


	std::string s_tex_name = "tex";
	std::vector<VkVertexInputBindingDescription> s_tex_b = { vkTools::initializers::vertexInputBindingDescription(0, sizeof(VertexT), VK_VERTEX_INPUT_RATE_VERTEX) };
	std::vector<VkVertexInputAttributeDescription> s_tex_a = { vkTools::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float))
	};
	std::vector<VkDescriptorSetLayoutBinding> s_tex_dslb = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
	};
	gEnv->pRenderer->addShader(gEnv->getAssetpath() + "shaders/tex.vert.spv", gEnv->getAssetpath() + "shaders/tex.frag.spv", &s_tex_name, s_tex_dslb, s_tex_b, s_tex_a);



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


	std::string s_font_name = "font";
	gEnv->pRenderer->addShader(gEnv->getAssetpath() + "shaders/font.vert.spv", gEnv->getAssetpath() + "shaders/font.frag.spv", &s_font_name, s_tex_dslb, s_tex_b, s_tex_a);

}

void RessourcesManager::_set_to_loaded()
{
	m_loaded = true;
}

bool RessourcesManager::_is_loaded()
{
	return m_loaded;
}
