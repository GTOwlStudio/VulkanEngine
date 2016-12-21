#pragma once

#include "guitype.h"

#include "Event.h"

#include "vulkan_header.h"

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

	virtual VkDeviceSize gSize(); //Get the size of the object for Allocate Graphic Memory, Vertices + Indices
	virtual VkDeviceSize gDataSize();
	virtual VkDeviceSize gIndicesSize();
	virtual void gData(void* arr); //Get the data of the object for fill memory VERTEX ONLY
	virtual void gIndices(void* arr); //Get the indices of the object INDICES ONLY
	VkDeviceSize getMemoryOffset();
	std::vector<uint32_t> getDescriptorsId();
	std::vector<VkDescriptorType> getDescriptorsType();

	std::string getClassName() const;
	virtual std::string getName() const;
	virtual rect2D getBoundary();
	virtual std::vector<Widget*> getChilds();
	//virtual void resize();

protected:
	std::string m_className;
	void addDescriptors(VkDescriptorType type);
	bool m_visible = false;
	std::string m_name;
	rect2D m_boundary;
	float m_depth = 0.0f;
	std::vector<Event> m_events;
	std::vector<Widget*> m_childs;
	
	VkDeviceSize m_memOffset;
	struct {
		std::vector<uint32_t> ids;
		std::vector<VkDescriptorType> types;
	} m_descriptors;
	
};

