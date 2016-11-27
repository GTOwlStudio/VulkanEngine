#pragma once

#include "guitype.h"

#include "Event.h"

/*
struct offset2D {
	float x, y;
	offset2D() : x(0.0f), y(0.0f) {}
	offset2D(float px, float py) : x(px), y(py) {}
};
struct extent2D {
	float width, height;
	extent2D() : width(0.0f), height(0.0f) {}
	extent2D(float w, float h) : width(w), height(h) {}
};

struct rect2D {
	offset2D offset;
	extent2D extent;
	rect2D(offset2D poffset, extent2D pextent) : offset(poffset), extent(pextent) {}
	rect2D(float x, float y, float w, float h) : offset(x, y), extent(w, h) {}
};*/


class Widget
{
public:
	Widget(std::string name, rect2D boundary);
	virtual ~Widget();
	virtual void addWidget(Widget* widget);

	virtual void move(offset2D pos);
	virtual void resize(extent2D newSize);
	virtual void setVisibility(bool visibility);

	virtual void addEvent(EventType type);

	virtual uint32_t gSize(); //Get the size of the object for Allocate Graphic Memory, Vertices + Indices
	virtual void gData(void* arr); //Get the data of the object for fill memory VERTEX ONLY
	virtual void gIndices(void* arr); //Get the indices of the object INDICES ONLY

	virtual std::string getName() const;
	virtual rect2D getBoundary();
	virtual std::vector<Widget*> getChilds();
	//virtual void resize();

protected:
	bool m_visible = false;
	std::string m_name;
	rect2D m_boundary;
	float m_depth = 0.0f;
	std::vector<Event> m_events;
	std::vector<Widget*> m_childs;

};

