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
	void update();
	void addWidget(Widget* widget);

protected:
	std::vector<Widget*> m_widgets;

	struct {
		glm::mat4 projection;
		SIndexedDrawInfo draw;
	} m_draw;


public:
	static uint32_t instance;

};

