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
	void loadDescriptorSets();

	void addDoubleSetting(std::string name, double value);
	//void addCreator(std::string name, std::function<void(mxml_node_t* t)> creator);//void(GUI::*function)(mxml_node_t* t));// std::function<void(mxml_node_t* t)> creator);
	void addCreator(std::string name, void (GUI::*&&creator)(mxml_node_t*t));// , void (GUI::*&&loader)(mxml_node_t *t));
	//void addLoader(std::string name, void (GUI::*&&loader)(mxml_node_t *t));
	void searchAndExecute(mxml_node_t* node);
	
	void creator_Panel(mxml_node_t *t);
	void creator_guilabel(mxml_node_t *t);
	//void checkNode();

	std::string m_renderPassName = "gui";

	std::vector<Widget*> m_widgets;

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
		size_t indicesOffsetId;
		size_t indicesSize = 0;
	} m_draw;

	

	std::string m_file;

	uint32_t test_texId;
	VkDescriptorImageInfo test_imgDescriptor;

	struct {

	} layoutInfo;

	struct {
		std::vector<double> doubleSettings;
		std::vector<std::string> doubleSettingsName;
	} m_settings;


	struct {
		std::vector<std::string> elementNames;
		std::vector<std::function<void(mxml_node_t *t)>> creators;
		std::vector<std::function<void(mxml_node_t* t)>> loaders;
		//std::vector<void(GUI::*)(mxml_node_t* t)> creators;
	} m_creators;

public:
	static uint32_t instance;

};

