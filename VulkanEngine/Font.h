#pragma once

#include <string>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>

#include "System.h"
#include "vulkanTools\vulkandebug.h"

struct character_info
{
	float ax; //x advance
	float ay;

	float bw; //bitmap width
	float bh;

	float w; //font width
	float h; //font height
	
	float bx; //X bearing
	float by; //Y bearing

	float tx; //x offset of glyph in texture coordinates
	float ty; //y offset of glyph in texture coordinates

};

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
	uint32_t m_size; // The Size took by the vertices and indices

	uint32_t m_atlas_width;
	uint32_t m_atlas_height;

	uint8_t* m_data; //this is the atlas in the buffer, a texture
	//void* m_verticesData; //data related to vertices and indices
	character_info* m_characterInfo;


	uint32_t m_texId;
	uint32_t bufferId = 0;

	SIndexedDrawInfo m_draw;

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



