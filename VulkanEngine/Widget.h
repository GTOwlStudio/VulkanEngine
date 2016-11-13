#pragma once
#include "guitools.h"
class Widget
{
public:
	Widget(std::string name, rect2D boundary);
	virtual ~Widget();
	virtual void addWidget(Widget* widget);
	//virtual void resize();

protected:
	std::string m_name;
	rect2D m_boundary;
	std::vector<Widget*> m_childs;

};

