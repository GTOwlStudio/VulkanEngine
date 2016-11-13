#pragma once
#include "guitools.h"
class Widget
{
public:
	Widget(std::string name, rect2D boundary);
	virtual ~Widget();
	virtual void addWidget(Widget* widget);

	virtual void move(offset2D pos);
	virtual void resize(extent2D newSize);

	virtual std::string getName() const;
	virtual rect2D getBoundary() const;
	virtual std::vector<Widget*> getChilds();
	//virtual void resize();

protected:
	std::string m_name;
	rect2D m_boundary;
	std::vector<Widget*> m_childs;

};

