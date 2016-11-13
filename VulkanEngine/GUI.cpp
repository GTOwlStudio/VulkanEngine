#include "GUI.h"

uint32_t GUI::instance = 0;


GUI::GUI()
{
	if (instance>=1) {
		assert("Multiple GUI created, must only have one");
	}
	instance += 1;
	
	addWidget(new Label("Sample", offset2D(50,50)));
	
}


GUI::~GUI()
{
	for (Widget* w : m_widgets) {
		delete w;
		w = 0;
	}
}

void GUI::load()
{
}

void GUI::addWidget(Widget * widget)
{
	m_widgets.push_back(widget);
}
