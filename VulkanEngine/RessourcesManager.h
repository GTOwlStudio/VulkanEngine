#pragma once

#include <list>
#include <algorithm>
#include "System.h"

class CFont;

class RessourcesManager
{
public:
	RessourcesManager();
	~RessourcesManager();
	CFont* getCFont(std::string fontname, uint32_t size);
	void addFont(std::string fontname, uint32_t size);
	bool exist(std::string fontname, uint32_t size);
protected:
	//std::list<CFont*/*, std::allocator_traits<CFont*>::pointer*/> m_fonts;
	std::list<CFont*> m_fonts;
	std::list<std::string> m_fontid; //name format <FamilyName><StyleName(firstcharacter)><FontSize>

};

