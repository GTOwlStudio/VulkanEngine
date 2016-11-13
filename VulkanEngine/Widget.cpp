#include "Widget.h"

Widget::Widget(std::string name, rect2D boundary) : m_name(name), m_boundary(boundary)
{
}

Widget::~Widget()
{
	for (Widget* w : m_childs) {
		delete w;
		w = 0;
	}
}

void Widget::addWidget(Widget * widget)
{
	m_childs.push_back(widget);
}
