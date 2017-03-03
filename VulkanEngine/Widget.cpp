#include "guitools.h"
#include "Widget.h"

Widget::Widget(std::string name, rect2D boundary, std::string text) : m_name(name), m_boundary(boundary), m_bufferId(), m_className("Widget"), m_text(text)
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

VkDeviceSize Widget::gSize()
{
	return VkDeviceSize();
}

VkDeviceSize Widget::gDataSize()
{
	return VkDeviceSize();
}

VkDeviceSize Widget::gIndicesSize()
{
	return VkDeviceSize();
}

void Widget::gData(void* arr)
{
}

void Widget::gIndices(void * arr)
{
}

size_t Widget::getBufferId()
{
	return m_bufferId;
}

size_t Widget::getDescriptorIdsSize()
{
	return m_descriptors.ids.size();
}

std::vector<uint32_t> Widget::getDescriptorsIds()
{
	return m_descriptors.ids;
}

std::vector<VkDescriptorType> Widget::getDescriptorsType()
{
	return m_descriptors.types;
}

std::string Widget::getClassName() const
{
	return m_className;
}

std::string Widget::getText() const
{
	return m_text;
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

void Widget::addDescriptors(VkDescriptorType type)
{
	m_descriptors.ids.push_back(gEnv->pRenderer->requestDescriptorSet(type, 1));
	m_descriptors.types.push_back(type);
}
