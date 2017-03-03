#include "nongraphicfont.h"



nongraphicfont::nongraphicfont(std::string path, uint32_t size) : m_fontSize(size)
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
	printf("Font name %s\n", static_cast<std::string>(m_face->style_name).c_str());
	FT_Set_Pixel_Sizes(m_face, 0, m_fontSize);

	uint32_t texture_width = 512;
	uint32_t texture_height = 0;

	uint32_t tmpWidth = 0;
	uint32_t tmpMaxHeight = 0;

	m_numOfCharacter = 256;

	m_characterInfo = new character_info[m_numOfCharacter];

	for (uint32_t i = 0; i<m_numOfCharacter; i++) {
		error = FT_Load_Char(m_face, i, FT_LOAD_RENDER);
		if (error) { printf("Error : while loading the character %c (%i)\n", i, i); }

		m_characterInfo[i].ax = static_cast<float>(slot->advance.x >> 6);
		m_characterInfo[i].ay = static_cast<float>(slot->advance.y >> 6);
		m_characterInfo[i].bx = static_cast<float>(slot->metrics.horiBearingX / 64);
		m_characterInfo[i].by = static_cast<float>(slot->metrics.horiBearingY / 64);
		m_characterInfo[i].w = static_cast<float>(slot->metrics.width / 64);
		m_characterInfo[i].h = static_cast<float>(slot->metrics.height / 64);
		m_characterInfo[i].bw = slot->bitmap.width;
		m_characterInfo[i].bh = slot->bitmap.rows;

		tmpWidth += slot->bitmap.width;
		tmpMaxHeight = std::max(tmpMaxHeight, slot->bitmap.rows);

		if (tmpWidth >= 512)
		{
			tmpWidth -= 512;
			texture_height += tmpMaxHeight;
			tmpMaxHeight = 0;
		}
	}

	m_atlas_width = texture_width;
	m_atlas_height = texture_height + tmpMaxHeight;

	m_data = new uint8_t[m_atlas_width*m_atlas_width];
	memset(m_data, 0, m_atlas_width*m_atlas_height);

	int lastX = 0;
	int lastY = 0;

	tmpMaxHeight = 0;


	uint32_t it = 0; //it = iterator
	for (uint32_t i = 0; i < m_numOfCharacter; i++) {
		//for (uint32_t i = 66; i < 67; i++) {
		FT_Load_Char(m_face, i, FT_LOAD_RENDER);

		if (lastX + slot->bitmap.width >= texture_width) {
			lastX = 0;
			lastY += tmpMaxHeight;
			tmpMaxHeight = 0;
		}

		m_characterInfo[i].tx = (float)lastX / m_atlas_width;
		m_characterInfo[i].ty = (float)lastY / m_atlas_height;
		for (uint32_t y = 0; y < slot->bitmap.rows; y++) {
			for (uint32_t x = 0; x < slot->bitmap.width; x++) {
				it = (lastY*m_atlas_width) + (y*m_atlas_width) + x + lastX;
				m_data[it] = slot->bitmap.buffer[(slot->bitmap.width)*y + x];
			}
		}

		tmpMaxHeight = std::max(tmpMaxHeight, slot->bitmap.rows);
		lastX += slot->bitmap.width/**slot->bitmap.rows*/;

	}

	m_fontname = static_cast<std::string>(m_face->family_name);
	m_stylename = static_cast<std::string>(m_face->style_name);

	FT_Done_Face(m_face);
	FT_Done_FreeType(m_lib);

	//gEnv->pMemoryManager->requestMemory(uint64_t(sizeof(float) * m_numOfCharacter*4)); //*4 cos a quad is 4 vertices
}


nongraphicfont::~nongraphicfont()
{
}

void nongraphicfont::getCharacterInfo(character_info * dst, uint32_t beg, uint32_t end)
{
	for (uint32_t i = beg; i < end; i++) {
		dst[i - beg] = m_characterInfo[i];
	}
}


uint32_t nongraphicfont::getNumOfCharacter() const
{
	return m_numOfCharacter;
}

uint32_t nongraphicfont::getFontSize() const
{
	return m_fontSize;
}

std::string nongraphicfont::getFontName() const
{
	return m_fontname;
}

std::string nongraphicfont::getStyleName() const
{
	return m_stylename;
}
