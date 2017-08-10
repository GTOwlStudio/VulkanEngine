#pragma once

#include <vector>
#include <assert.h>

#include "Framebuffer.h"
#include "Widget.h"
#include "guilabel.h"
#include "Panel.h"
#include "XMLParser.h"
#include "vulkanTools\vulkanTextureLoader.hpp"
#include <functional>
//#include <glm\glm.hpp>

class CSystem;
class CFramebuffer;
class GUI;
//struct SIndexedDrawInfo;

/*
struct SGuiSettings
{
	std::vector<double> doubleSettings;
	std::veto
};*/


struct XMLWidget { //XML Widget description
	std::string name = "";
	float x;
	float y;
	float width;
	float height;
	mxml_node_t* node = nullptr;
	mxml_node_t* parent = nullptr;
	std::string parentName = "";
	XMLWidget() : name(""), x(-1), y(-1), width(-1), height(-1), parent(nullptr) {

	}
	XMLWidget(std::string param_name, float w, float h, float posx, float posy, mxml_node_t* param_parent = nullptr) : name(param_name), x(posx), y(posy), width(w), height(h), parent(param_parent) {
	}

	offset2D getPos() {
		return offset2D(x,y);
	}

	extent2D getBounds() {
		return extent2D(width, height);
	}

};

class GUI
{
public:
	GUI(std::string file);
	~GUI();
	void load(); //load vulkan object
	void update();
	void updateUniformBuffer();
	void addWidget(Widget* widget);
	void getSettings(std::string sName);

protected:
	std::string getNextName(std::string prefix = "");
	offset2D getNextPosition();
	void loadCreator();
	void loadFromXml(std::string file);
	void loadGuiSettings(std::string file);
	void loadWidgets();
	void loadWidgets_dev();
	void reorderWidgets();
	void loadDescriptorSets();

	XMLWidget getXMLWidgetInfo(mxml_node_t * node);
	XMLWidget fillXMLWidgetInfo(mxml_node_t * node, Widget* w); //Tools for logical representation

	XMLWidget findXMLWidgetInfo(mxml_node_t* node);

	void addDoubleSetting(std::string name, double value);
	//void addCreator(std::string name, std::function<void(mxml_node_t* t)> creator);//void(GUI::*function)(mxml_node_t* t));// std::function<void(mxml_node_t* t)> creator);
	void addCreator(std::string name, void (GUI::*&&creator)(mxml_node_t*t));// , void (GUI::*&&loader)(mxml_node_t *t));
	//void addLoader(std::string name, void (GUI::*&&loader)(mxml_node_t *t));
	void searchAndExecute(mxml_node_t* node);
	
	void creator_Panel(mxml_node_t *t);
	void creator_guilabel(mxml_node_t *t);
	void creator_menu(mxml_node_t *t);
	//void checkNode();

	std::string m_renderPassName[2] = {"gui_clear", "gui"}; //2 renderPasses, one for clearing the framebuffer and the other one just load the content of the first one

	std::vector<Widget*> m_widgets;
	std::vector<XMLWidget> m_logicWidgets; //Logical representtion of the widget

	struct {
		struct {
			glm::mat4 projection;
		} UBO; //Uniform Buffer Object
		size_t UBO_bufferId;
		//VkDeviceSize gOffset[1];
		std::vector<VkDeviceSize> gOffset;
		std::vector<std::vector<VkDeviceSize>> gOffsets;
		std::vector<SIndexedDrawInfo> draws;
		std::vector<uint32_t> descriptorSetId;
		std::vector<VkDescriptorType> descriptorSetTypes;
		
		CFramebuffer* offscreen;
		
		size_t vertClrId;		//Virtual Colored Vertex Buffer Id
		size_t vertClrSize = 0; //Virtual Colored Vertex Buffer Size
		size_t vertTexId;		//Virtual Textured Vertex Buffer Id
		size_t vertTexSize = 0; //Virtual Textured Vertex Buffer Size
		size_t indicesOffsetId;
		size_t indicesSize = 0;
	} m_draw;

	

	std::string m_file;

	uint32_t test_texId;
	VkDescriptorImageInfo test_imgDescriptor;

	uint32_t m_fontSize = 15;
	std::string m_fontName = "segoeui";

	struct {

	} layoutInfo;

	struct {
		std::vector<double> doubleSettings;
		std::vector<std::string> doubleSettingsName;
		
	} m_settings;

	std::string m_xmlBase = "";
	mxml_node_t* m_xmlTop = nullptr;

	std::vector<std::string> m_elementNames;
	std::vector<uint32_t> m_elementsCount;
	std::vector<std::function<void(mxml_node_t *t)>> m_creators;
	std::vector<std::function<void(mxml_node_t* t)>> m_loaders;
	//std::vector<void(GUI::*)(mxml_node_t* t)> creators;

public:
	static uint32_t instance;

};

