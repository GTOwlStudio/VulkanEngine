#include "GUI.h"
#include "Framebuffer.h"


uint32_t GUI::instance = 0;


GUI::GUI(std::string file) : m_file(file)
{
	if (instance>=1) {
		assert("Multiple GUI created, must only have one");
	}
	instance += 1;

	loadCreator();
	loadGuiSettings("GuiSettings.xml");
	loadFromXml(m_file);
	
	//addWidget(new Label("Sample", offset2D(50,50)));

	//addWidget(new Panel("Panel", rect2D(100, 100, 100, 100)));
	m_draw.projection = glm::ortho<float>(0.0f, (float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f); //GUI coherent 
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
	
	printf("%i object to be load\n", static_cast<int>(m_widgets.size()));
	loadWidgets();
	//gEnv->pRenderer->addRenderPass("gui");
	//m_draw.offscreen = gEnv->pRenderer->addOffscreen("gui");

}

void GUI::update()
{
}

void GUI::addWidget(Widget * widget)
{
	m_widgets.push_back(widget);
}

void GUI::getSettings(std::string sName)
{
}

std::string GUI::getNextName(std::string prefix)
{
	return prefix + std::to_string(m_widgets.size());
}

offset2D GUI::getNextPosition()
{
	offset2D np;
	for (size_t i = 0; i < m_widgets.size();i++) {
		np.x = m_widgets[i]->getBoundary().offset.x + m_widgets[i]->getBoundary().extent.width;
		np.y = m_widgets[i]->getBoundary().offset.y + m_widgets[i]->getBoundary().extent.height;
		if (static_cast<uint32_t>(np.x)>=gEnv->pSystem->getWidth()) {
			np.x = 0;
		}
	}
	return np;
}

void GUI::loadCreator()
{
	//addCreator("Panel", &GUI::creator_Panel);
	addCreator("Panel", &GUI::creator_Panel);
	//addCreator("Panel", std::bind(&GUI::creator_Panel, this, std::placeholders::_1));
	//addCreator("Panel", [](mxml_node_t *t) {printf("PanelConstructor %s\n"); });
}

void GUI::loadFromXml(std::string file)
{
	XMLParser parser(file);

	double top = static_cast<double>(gEnv->pSystem->getHeight());
	mxml_node_t* node = parser.getTop();
	while (node!=nullptr) {
		searchAndExecute(node);
		node = parser.getNextElement(node);
	}

}

void GUI::loadGuiSettings(std::string file)
{
	XMLParser parser(file);
	std::vector<std::string> doubleSettings = {"DefaultMenuBarWidth", "DefaultHorizontalSpacerSize", "DefaultVerticalSpacerSize"};
	std::string tmp = parser.getValue("DefaultMenuBarWidth", "value");
	for (size_t i = 0; i < doubleSettings.size();i++) {
		tmp = parser.getValue(doubleSettings[i], "value");
		if (tmp != "empty:string") {
			addDoubleSetting(doubleSettings[i], std::stod(tmp));
		}
	}
}

void GUI::loadWidgets()
{
	for (size_t i = 0; i < m_widgets.size();i++) {
		if (m_widgets[i]->getClassName()=="Panel") {
			printf("PANEL\n");
			Widget* tw = m_widgets.back(); //tmp widget
			VertexC* tmpV = new VertexC[meshhelper::QUAD_VERTICES_COUNT];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
			tw->gData(tmpV);
			tw->gIndices(tmpI);
			printf("%i ",(int)tw->getMemoryOffset());
			printf("%i %i\n", (int)tw->gDataSize(), (int)(tw->gDataSize() + tw->getMemoryOffset()));
			printf("%i %i\n", (int)tw->gIndicesSize(), (int)(tw->gDataSize() + tw->gIndicesSize() + tw->getMemoryOffset()));
			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tw->getMemoryOffset(), tmpV);
			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gIndicesSize(), tw->getMemoryOffset() + tw->gDataSize(), tmpI);

			SIndexedDrawInfo drawInfo = {};
			drawInfo.bindDescriptorSets(0, 0, nullptr);
			//drawInfo.bindPipeline(gEnv->);

			m_draw.draws.push_back(drawInfo);
		}
	}
}

void GUI::addDoubleSetting(std::string name, double value)
{
	m_settings.doubleSettingsName.push_back(name);
	m_settings.doubleSettings.push_back(value);
}
/*
void GUI::addCreator(std::string name, std::function<void(mxml_node_t* t)> creator) //void (GUI::*creator)(mxml_node_t* t))//std::function<void(mxml_node_t* t)> creator)
{
	m_creators.elementNames.push_back(name);
	m_creators.creators.push_back(creator);
}*/

void GUI::addCreator(std::string name, void(GUI::*&& _Func)(mxml_node_t *t))// , void (GUI::*&&loader)(mxml_node_t *t))
{
	m_creators.elementNames.push_back(name);
	m_creators.creators.push_back(std::bind(_Func, this, std::placeholders::_1));
}


void GUI::searchAndExecute(mxml_node_t * node)
{
	XMLParser parser(m_file);
	for (size_t i = 0; i < m_creators.creators.size();i++) {
		if (parser.getXData(node).elementName==m_creators.elementNames[i]) {
			m_creators.creators[i](node);
		}
	}
}

void GUI::creator_Panel(mxml_node_t * t)
{
	printf("Panel creator : %s\n", m_file.c_str());
	printf("%f %f\n",getNextPosition().x, getNextPosition().y);
	addWidget(new Panel("panel", rect2D(getNextPosition(), extent2D(100, 20))));


	printf("%f %f\n", getNextPosition().x, getNextPosition().y);
}
