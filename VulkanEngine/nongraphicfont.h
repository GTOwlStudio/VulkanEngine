#pragma once

#include <string>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>


#include "fonttype.h"

class nongraphicfont
{
public:
	nongraphicfont(std::string path, uint32_t size);
	~nongraphicfont();

	void getCharacterInfo(character_info* dst, uint32_t beg = 0, uint32_t end = 255);
	uint32_t getNumOfCharacter() const;
	uint32_t getFontSize() const;
	std::string getFontName() const;
	std::string getStyleName() const;

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

};

