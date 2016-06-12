#pragma once
#include <iostream>
#include <vector>



#include "TCore.h"
#include <vulkan\vulkan.h>

#include "guiButton.h"
#include "guiTools.h"
#include "vulkantools.h"
#include "Input.h"

class TCore;

class TGui
{
public:
	TGui(TCore &Core, Input &input);
	//TGui(TCore &core);
	~TGui();

	void load();

	void updateLogic();
	void update();

protected:

	struct
	{
		VkBuffer buf;
		VkDeviceMemory mem;
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescription;

	} vertices;

	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		uint32_t count;
	} indices;

	struct {
		glm::mat4 projection;
	}uboVS;

	vkTools::UniformData uniformDataVS;

	struct {
		VkPipeline gui;
	} pipelines;


	VkPipelineLayout _pipelineLayout;
	VkDescriptorSet _descriptorSet;
	VkDescriptorSetLayout _descriptorSetLayout;

	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

	Input &_input;
	TCore &_core;

	std::vector<GData> _data;
	std::vector<uint32_t> _indices;
	std::vector<ITGuiObject*> _objects;




	void generateGuiCoord();
	void initObjects();

	void setupVertexDescription();
	void prepareUniformBuffers();
	void setupDescriptorLayout();
	void preparePipelines();
	void setupDescriptorPool();
	void setupDescriptorSet();
	void buildCommandBuffers();

	void setUniformBuffers();

	void destroyObjects();
};

