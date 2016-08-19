#include "Font.h"



CFont::CFont(std::string path, uint32_t size) : m_fontSize(size)
{
	int error;
	error = FT_Init_FreeType(&m_lib);

	if (error) { printf("ERROR : Couldn't load Freetype\n"); }

	error = FT_New_Face(m_lib, path.c_str(), 0, &m_face);

	if (error == FT_Err_Unknown_File_Format) {
		printf("ERROR : Unknow file format %s\n", path.c_str());
	}

	if (error) {
		printf("ERROR : code %i while loading the font\n", error);
	}

	FT_GlyphSlot slot = m_face->glyph;

	FT_Set_Pixel_Sizes(m_face, 0, m_fontSize );
	
	uint32_t texture_width = 512;
	uint32_t texture_height = 0;

	uint32_t tmpWidth = 0;
	uint32_t tmpMaxHeight = 0;

	m_numOfCharacter = 256;

	m_characterInfo = new character_info[m_numOfCharacter];

	for (uint32_t i = 0;i<m_numOfCharacter;i++){
		error = FT_Load_Char(m_face, i, FT_LOAD_RENDER);
		if (error) { printf("Error : while loading the character %c (%i)\n", i, i); }

		m_characterInfo[i].ax = slot->advance.x >> 6;
		m_characterInfo[i].ay = slot->advance.y >> 6;
		m_characterInfo[i].bx = slot->metrics.horiBearingX /64;
		m_characterInfo[i].by= slot->metrics.horiBearingY /64;
		m_characterInfo[i].w = slot->metrics.width / 64;
		m_characterInfo[i].h = slot->metrics.height /64;
		m_characterInfo[i].bw = slot->bitmap.width;
		m_characterInfo[i].bh = slot->bitmap.rows;

		tmpWidth += slot->bitmap.width;
		tmpMaxHeight = std::max(tmpMaxHeight, slot->bitmap.rows);

		if (tmpWidth>=512)
		{
			tmpWidth -= 512;
			texture_height += tmpMaxHeight;
			tmpMaxHeight = 0;
		}
	}

	m_atlas_width = texture_width;
	m_atlas_height = texture_height + tmpMaxHeight;

	m_data = new uint8_t[m_atlas_width*m_atlas_width];
	

	int lastX = 0;
	int lastY = 0;

	tmpMaxHeight = 0;

	/*std::ofstream stream;
	stream.open("output.txt");*/

	for (uint32_t i = 0; i < m_numOfCharacter;i++) {
		FT_Load_Char(m_face, i, FT_LOAD_RENDER);

		if (lastX + slot->bitmap.width >= texture_width) {
			lastX = 0;
			lastY += tmpMaxHeight;
			tmpMaxHeight = 0;
		}

		m_characterInfo[i].tx = (float) lastX/ m_atlas_width;
		m_characterInfo[i].ty = (float)lastY / m_atlas_height;
		for (uint32_t y = 0; y < slot->bitmap.rows; y++) {
			for (uint32_t x = 0; x < slot->bitmap.width;x++) {
				m_data[(y+lastY)*m_atlas_width+ x + lastX] = slot->bitmap.buffer[(slot->bitmap.width)*y+x];
				//stream << (uint32_t)m_data[(y+lastY)*m_atlas_width + x+ lastX] << "\t";
			}
			//stream << std::endl;
		}
		tmpMaxHeight = std::max(tmpMaxHeight, slot->bitmap.rows);
		lastX += slot->bitmap.width;
		
	}

//	stream.close();

	printf("w:%i h:%i\n", texture_width, texture_height+tmpMaxHeight);

	gEnv->pMemoryManager->requestMemory(uint64_t(sizeof(float) * m_numOfCharacter*4)); //*4 cos a quad is 4 vertices

}


CFont::~CFont()
{
	delete[] m_data;
	delete[] m_characterInfo;
	m_data = 0;
	FT_Done_Face(m_face);
	FT_Done_FreeType(m_lib);
}

void CFont::load()
{
	VkImageCreateInfo imageInfo = vkTools::initializers::imageCreateInfo();
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8_UNORM;
	imageInfo.extent.width = m_atlas_width;
	imageInfo.extent.height = m_atlas_height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	gEnv->pRenderer->createTexture(&m_texId, imageInfo, m_data, m_atlas_width, m_atlas_height);
	//request buffer for a quad
	//request pipeline and shader

/*	VkGraphicsPipelineCreateInfo pipelineCreateInfo = 
		vkTools::initializers::pipelineCreateInfo();
	gEnv->pRenderer->addGraphicPipeline();*/
	printf("tex id : %i\n", m_texId);

}

uint32_t CFont::getNumOfCharacter() const
{
	return m_numOfCharacter;
}