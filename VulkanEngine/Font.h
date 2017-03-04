#pragma once

#include <string>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>

#include "System.h"
#include "vulkanTools\vulkandebug.h"

#include "fonttype.h"

//struct character_info
//{
//	float ax; //x advance
//	float ay;
//
//	uint32_t bw; //bitmap width
//	uint32_t bh;
//
//	float w; //font width
//	float h; //font height
//	
//	float bx; //X bearing
//	float by; //Y bearing
//
//	float tx; //x offset of glyph in texture coordinates
//	float ty; //y offset of glyph in texture coordinates
//
//};

class CFont
{
public:
	CFont(std::string path, uint32_t size);
	~CFont();

	
	void load();
	void getCharacterInfo(character_info* dst, uint32_t beg = 0, uint32_t end = 255);
	uint32_t getNumOfCharacter() const;
	uint32_t getFontSize() const;
	std::string getFontName() const;
	std::string getStyleName() const;
	VkDescriptorImageInfo getDescriptorImageInfo();
	uint32_t getAtlasWidth() const;
	uint32_t getAtlasHeight() const;


protected:
	FT_Library m_lib;
	FT_Face m_face;
	uint32_t m_fontSize;
	uint32_t m_numOfCharacter;
	uint32_t m_size; // The Size took by the vertices and indices

	uint32_t m_atlas_width;
	uint32_t m_atlas_height;

	uint8_t* m_data; //this is the atlas in the buffer, a texture
	//void* m_verticesData; //data related to vertices and indices
	character_info* m_characterInfo;
	std::string m_fontname;
	std::string m_stylename;

	uint32_t m_texId;
	uint32_t bufferId = 0;
	size_t m_vBufferId = 0;
	//VkDeviceSize m_bufferOffset; //The Offset In The Buffer

	SIndexedDrawInfo m_draw;
	VkDeviceSize m_gOffsets[1];
	uint32_t m_descriptorSetId;
	std::vector<VkFramebuffer> m_fb;
	VkDescriptorImageInfo m_imageDescriptor;

	//#dev

	//vkTools::UniformData m_ud; //uniform data
/*	vk::VulkanDevice *vulkanDevice;
	VkQueue queue;
	VkFormat colorFormat;
	VkFormat depthFormat;
	
	uint32_t *frameBufferWidth;
	uint32_t *frameBufferHeight;

	VkSampler sampler;
	VkImage image;
	VkImageView view;*/
};



