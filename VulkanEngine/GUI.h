#pragma once
#include <vector>
#include <assert.h>
#include "Widget.h"
#include "Label.h"


class GUI
{
public:
	GUI();
	~GUI();
	void load(); //load vulkan object
	void addWidget(Widget* widget);

protected:
	std::vector<Widget*> m_widgets;

public:
	static uint32_t instance;

};

