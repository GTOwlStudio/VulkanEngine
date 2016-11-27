#pragma once

#include <vector>
#include <assert.h>

#include "Framebuffer.h"
#include "Widget.h"
#include "guilabel.h"
#include "Panel.h"
//#include <glm\glm.hpp>

class CSystem;
class CFramebuffer;
class GUI;
//struct SIndexedDrawInfo;


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
		CFramebuffer* offscreen;
	} m_draw;


public:
	static uint32_t instance;

};

