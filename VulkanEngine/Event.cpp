#include "Event.h"



Event::Event(EventType eventType) : m_eventType(eventType)
{
}


Event::~Event()
{
}

void Event::process()
{
	m_processed = true;
}
