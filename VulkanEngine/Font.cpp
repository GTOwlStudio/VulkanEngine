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

	//gEnv->pMemoryManager->requestMemory(uint64_t(sizeof(float) * m_numOfCharacter*4)); //*4 cos a quad is 4 vertices

	m_size = static_cast<uint32_t>((sizeof(float) * 4*2) + (sizeof(uint32_t) * 6)) * m_numOfCharacter; // For a character there is 4 vertices, a vertices got 2 float(x,y) and there is 6 indices by character

	gEnv->pMemoryManager->requestMemory(m_size);
	gEnv->pMemoryManager->requestMemory(sizeof(VertexT)*4+(sizeof(uint32_t)*6));

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

	uint32_t charMem = sizeof(float)*8;//Size of a chracter in memory
	char* viData = new char[m_size]; //Vertex and Indices data tmp, all the vertex then all the indices

	char* charArray = new char[charMem];

	uint32_t offset = 0;

	struct{
		float tx; // X
		float ty; // Y
		float tw; // X + W
 		float th; // Y + H
	} quad;

	uint32_t indices[6] = {0,1,2,0,2,3};




	vkDebug::stringToFile("", "output.txt", false);
	vkDebug::stringToFile("", "outputc.txt", false);

	for (uint32_t i = 0; i < m_numOfCharacter;i++) {

		quad.tx = m_characterInfo[i].tx;
		quad.ty = m_characterInfo[i].ty;
		quad.tw = quad.tx + m_characterInfo[i].w;
		quad.th = quad.ty + m_characterInfo[i].h;

		memcpy(charArray  , &quad.tx, sizeof(float));
		memcpy(sizeof(float) + charArray  , &quad.ty, sizeof(float));

		memcpy(2 * sizeof(float) + charArray  , &quad.tw, sizeof(float));
		memcpy(3 * sizeof(float) + charArray  , &quad.ty, sizeof(float));

		memcpy(4 * sizeof(float) + charArray  , &quad.tw, sizeof(float));
		memcpy(5 * sizeof(float) + charArray  , &quad.th, sizeof(float));

		memcpy(6 * sizeof(float) + charArray  , &quad.tx, sizeof(float));
		memcpy(7 * sizeof(float) + charArray  , &quad.th, sizeof(float));

		//memcpy(256*sizeof(float) + charArray+(6*sizeof(uint32_t)), indices, sizeof(uint32_t)*6);

		memcpy(viData + (charMem*i), charArray, charMem);
		memcpy(viData + (256*charMem)+(i*6*sizeof(uint32_t)), indices, 6*sizeof(uint32_t));
		//printf("%i %i %i size = %i\n",i, (charMem*i),8*sizeof(float)+(charMem*i), m_size);

//		vkDebug::arrayToFile(charArray, charMem, "outputc.txt", true, true);

	/*	vkDebug::stringToFile(std::to_string(i)+"\t", "output.txt", true);
		vkDebug::arrayToFile(viData+(charMem*i), charMem, "output.txt", true, true);*/
		
	}
	//printf("%i\n", 255, );
	//vkDebug::arrayToFile(viData, charMem*m_numOfCharacter, "output.txt");
	vkDebug::arrayToFile(viData, m_size, "output.txt");
//	vkDebug::stringToFile("\0", "outputc.txt", true);

	gEnv->pRenderer->bufferSubData(gEnv->bbid, m_size, 0, viData);

	delete[] viData;
	delete[] charArray;

	//Prepare the render of the full texture
		//Prepare the vertex for the texture
	std::array<VertexT, 4> coords = { VertexT(-1.0f, 1.0f, 0.1f, 0.0f,1.0f), VertexT(1.0f,1.0f,0.1f,1.0f,1.0f), VertexT(1.0f,-1.0f,0.1f,1.0f,0.0f), VertexT(-1.0f,-1.0f,0.1f,0.0f,0.0f) };
	uint32_t gIndices[6] = {0,1,2, 0,2,3};


	gEnv->pRenderer->bufferSubData(gEnv->bbid, sizeof(VertexT)*4, m_size, coords.data());
	gEnv->pRenderer->bufferSubData(gEnv->bbid, sizeof(uint32_t)*6, m_size + sizeof(VertexT)*4, gIndices);
		//request pipeline and shader
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("texture")->getPipelineLayout(),
										gEnv->pRenderer->getRenderPass("main"),
										0,
										VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										VK_POLYGON_MODE_FILL,
										2,
										gEnv->pRenderer->getShader("texture")->getShaderStagesPtr(),
										gEnv->pRenderer->getShader("texture")->getInputState(), 
										"texture");
	printf("tex id : %i\n", m_texId);

	m_draw = {};
	m_draw.bindDescriptorSets(gEnv->pRenderer->getShader("texture")->getPipelineLayout(), 1, gEnv->pRenderer->getShader("texture")->getDescriptorSetPtr());
	m_draw.bindPipeline(gEnv->pRenderer->getPipeline("texture"));
	VkDeviceSize gOffsets[1] = { m_size };
	m_draw.bindVertexBuffers(gEnv->pRenderer->getBuffer(gEnv->bbid), 1, gOffsets);
	m_draw.bindIndexBuffer(gEnv->pRenderer->getBuffer(gEnv->bbid), m_size+(sizeof(VertexT) * 4), VK_INDEX_TYPE_UINT32);
	m_draw.drawIndexed(6, 1, 0, 0, 0);
	//m_draw.bindDescriptorSets();

}

uint32_t CFont::getNumOfCharacter() const
{
	return m_numOfCharacter;
}
