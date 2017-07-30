#pragma once

#include "guitype.h"

#include "Event.h"

#include "vulkan_header.h"

#include <limits>

struct XMLWidget { //XML Widget description
	std::string name = "";
	float x;
	float y;
	float width;
	float height;
	XMLWidget() : name(""), x(-1), y(-1), width(-1), height(-1) {

	}
	XMLWidget(std::string param_name, float w, float h, float posx, float posy) : name(param_name),x(posx),y(posy),width(w),height(h){
	}
};

class Widget
{
public:
	Widget(std::string name, rect2D boundary, std::string text="");
	virtual ~Widget();
	virtual void addWidget(Widget* widget);

	virtual void move(offset2D pos);
	virtual void resize(extent2D newSize);
	virtual void setVisibility(bool visibility);
	virtual void setDepth(float depth);
	virtual void setName(std::string name);
	virtual void setBufferId(size_t id);

	virtual void addEvent(EventType type);

	virtual VkDeviceSize gSize(); //Get the size of the object for Allocate Graphic Memory, Vertices + Indices
	virtual VkDeviceSize gDataSize();
	virtual VkDeviceSize gIndicesSize();
	virtual void gData(void* arr); //Get the data of the object for fill memory VERTEX ONLY
	virtual void gIndices(void* arr); //Get the indices of the object INDICES ONLY
	size_t getBufferId();
	size_t getDescriptorIdsSize();
	std::vector<uint32_t> getDescriptorsIds();
	std::vector<VkDescriptorType> getDescriptorsType();

	std::string getClassName() const;
	std::string getText() const;
	virtual std::string getName() const;
	virtual rect2D getBoundary();
	virtual std::vector<Widget*> getChilds();
	//virtual void resize();

protected:
	std::string m_className;
	void addDescriptors(VkDescriptorType type);
	bool m_visible = false;
	std::string m_name;
	std::string m_text;
	rect2D m_boundary;
	float m_depth = 0.0f;
	std::vector<Event> m_events;
	std::vector<Widget*> m_childs;
	
	size_t m_bufferId;
	struct {
		std::vector<uint32_t> ids;
		std::vector<VkDescriptorType> types;
	} m_descriptors;
	
};

