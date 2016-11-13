#pragma once
#include <functional>
class Action
{
public:
	Action(std::function<void(void)> fn);
	~Action();
	void perform();
protected:
	std::function<void(void)> m_action;

};

