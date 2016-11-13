#include "Event.h"



Event::Event()
{
}


Event::~Event()
{
}

void Event::process()
{
	m_processed = true;
}
