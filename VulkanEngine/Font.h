#pragma once

#include <string>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>

#include "System.h"
#include "vulkanTools\vulkandebug.h"

class CFont
{
public:
	CFont(std::string path, uint32_t size);
	~CFont();

	void load();

	uint32_t getNumOfCharacter() const;


protected:
	FT_Library m_lib;
	FT_Face m_face;
	uint32_t m_fontSize;
	uint32_t m_numOfCharacter;

	uint32_t m_atlas_width;
	uint32_t m_atlas_height;

	uint8_t* m_data;

	uint32_t m_texId;


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



