#include "GUI.h"
#include "Framebuffer.h"


uint32_t GUI::instance = 0;


GUI::GUI()
{
	if (instance>=1) {
		assert("Multiple GUI created, must only have one");
	}
	instance += 1;
	
	addWidget(new Label("Sample", offset2D(50,50)));
	addWidget(new Panel("Panel", rect2D(100, 100, 100, 100)));
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
	gEnv->pRenderer->addRenderPass("gui");
	m_draw.offscreen = gEnv->pRenderer->addOffscreen("gui");
}

void GUI::update()
{
}

void GUI::addWidget(Widget * widget)
{
	m_widgets.push_back(widget);
}
