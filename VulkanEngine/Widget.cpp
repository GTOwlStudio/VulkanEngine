#include "guitools.h"
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

void Widget::move(offset2D pos)
{
	m_boundary.offset = pos;
}

void Widget::resize(extent2D newSize)
{
	m_boundary.extent = newSize;
}

void Widget::setVisibility(bool visibility)
{
	if (m_visible!=visibility) {
		m_visible = visibility;
		if (visibility) {
			addEvent(EVENT_TYPE_WIDGET_SET_VISIBLE);
		}
		else {
			addEvent(EVENT_TYPE_WIDGET_SET_UNVISIBLE);
		}
	}
}

void Widget::addEvent(EventType type)
{
	m_events.push_back(Event(type));
}

uint32_t Widget::gSize()
{
	return 0;
}

void Widget::gData(void* arr)
{
}

void Widget::gIndices(void * arr)
{
}

std::string Widget::getName() const
{
	return m_name;
}

rect2D Widget::getBoundary()
{
	return m_boundary;
}

std::vector<Widget*> Widget::getChilds()
{
	return m_childs;
}
