#include "Label.h"



Label::Label(std::string text, rect2D boundary, std::string fontname, uint32_t fsize) : m_text(text), m_font(*gEnv->pRessourcesManager->getCFont(fontname, fsize)), Widget(text,  boundary)
{
}

Label::Label(std::string text, offset2D position, std::string fontname, uint32_t fsize) : 
	Label(text, rect2D(position, extent2D(0.0f,0.0f)), fontname, fsize)
{

}


Label::~Label()
{
}
