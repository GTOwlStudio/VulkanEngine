#pragma once

enum EventType {
	EVENT_TYPE_WIDGET_SET_VISIBLE, //Put it in the next render 
	EVENT_TYPE_WIDGET_SET_UNVISIBLE, //Put it off the next render
	EVENT_TYPE_WIDGET_RESIZED, //update
	EVENT_TYPE_WIDGET_MOVED,
	EVENT_TYPE_WIDGET_ENTERED,
	EVENT_TYPE_WIDGET_CHILD_ADDED,
	EVENT_TYPE_WIDGET_CHILD_REMOVED,
	EVENT_TYPE_WIDGET_COLOR_CHANGED,
	EVENT_TYPE_BUTTON_CLICKED
};

class Event
{
public:
	Event();
	~Event();
	void process();

protected:
	EventType m_eventType;
	bool m_processed = false;

};

