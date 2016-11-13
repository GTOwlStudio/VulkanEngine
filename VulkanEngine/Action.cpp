#include "Action.h"

Action::Action(std::function<void(void)> fn) : m_action(fn)
{
}

Action::~Action()
{
}

void Action::perform()
{
	m_action();
}
