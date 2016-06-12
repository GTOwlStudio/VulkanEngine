#include "TGui.h"



TGui::TGui(TCore &core, Input &input) : _input(input), _data(), _objects(), _core(core)
{
}


TGui::~TGui()
{

	/*if (_descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool();
	}*/

/*	vkDestroyPipeline(_core.getTEngine()->getDevice(), pipelines.gui, nullptr);

	vkDestroyPipelineLayout(_core.getTEngine()->getDevice(), _pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(_core.getTEngine()->getDevice(), _descriptorSetLayout, nullptr);

	

	vkDestroyBuffer(_core.getTEngine()->getDevice(), uniformDataVS.buffer, nullptr);
	vkFreeMemory(_core.getTEngine()->getDevice(), uniformDataVS.memory, nullptr);

	destroyObjects();*/
	
}

void TGui::load()
{
	initObjects();
	generateGuiCoord();
	setupVertexDescription();
	prepareUniformBuffers();

}

void TGui::updateLogic()
{
	for (size_t i = 0; i < _objects.size(); i++) {
		_objects[i]->updateLogic();
	}
}

void TGui::generateGuiCoord()
{
	size_t pSize = 0;//Taille precedente du tableau contenant les indices
	for (size_t i = 0; i < _objects.size(); i++) {
		//_data.reserve(_objects[i]->getCoords().size());
		for (size_t j = 0; j < _objects[i]->getData().size(); j++) {
			_data.push_back(_objects[i]->getData()[j]);
		}
		for (size_t j = 0; j < _objects[i]->getIndices().size(); j++) {
			_indices.push_back(_objects[i]->getIndices()[j] + (i*pSize));
		}
		pSize = _objects[i]->getIndices().size();

	}
	
	_core.getTEngine()->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		_data.size() * sizeof(GData),
		_data.data(),
		&vertices.buf,
		&vertices.mem);

	_core.getTEngine()->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		_indices.size()*sizeof(uint32_t),
		_indices.data(),
		&indices.buf,
		&indices.mem);

}

void TGui::initObjects()
{
	if (_objects.size() > 0) {
		destroyObjects();
	}

	_objects.push_back(new guiButton(10.0f, 10.0f, 10.0f, 10.0f, 0.1f, _input));

}

void TGui::setupVertexDescription()
{
	vertices.bindingDescription.resize(1);
	vertices.bindingDescription[0] = vkTools::initializers::vertexInputBindingDescription(
		1,
		sizeof(GData),
		VK_VERTEX_INPUT_RATE_VERTEX);

	vertices.attributeDescription.resize(2);
	vertices.attributeDescription[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			1,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);
	vertices.attributeDescription[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			1,
			1,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			sizeof(float) * 3);

	vertices.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescription.size();
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescription.data();
	vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescription.size();
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescription.data();
}

void TGui::prepareUniformBuffers()
{
		_core.getTEngine()->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		sizeof(uboVS),
		&uboVS,
		&uniformDataVS.buffer,
		&uniformDataVS.memory,
		&uniformDataVS.descriptor);
}

void TGui::setupDescriptorLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		//Binding 0 : for Vertex Shader uniform buffer
		vkTools::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT,
		0)
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vkTools::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());

	VkResult err = vkCreateDescriptorSetLayout(_core.getTEngine()->getDevice(), &descriptorLayout, NULL, &_descriptorSetLayout);
	assert(!err);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vkTools::initializers::pipelineLayoutCreateInfo(&_descriptorSetLayout, 1);

	err = vkCreatePipelineLayout(_core.getTEngine()->getDevice(), &pPipelineLayoutCreateInfo, nullptr, &_pipelineLayout);
	assert(!err);
}

void TGui::preparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vkTools::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachemntSatte =
		vkTools::initializers::pipelineColorBlendAttachmentState(0xf,VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vkTools::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachemntSatte);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState = vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), dynamicStateEnables.size(), 0);

	std::string assetPath = _core.getTEngine()->getAssetPath();

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = _core.getTEngine()->loadShader(assetPath + "shaders/gui_solid.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = _core.getTEngine()->loadShader(assetPath + "shaders/gui_solid.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(_pipelineLayout, _core.getTEngine()->getRenderPass(),0);

	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();

	VkResult err = vkCreateGraphicsPipelines(_core.getTEngine()->getDevice(), _core.getTEngine()->getPipelineCache(), 1, &pipelineCreateInfo, nullptr, &pipelines.gui);
	assert(!err);
}

void TGui::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = 
	{
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 1);

	VkResult vkRes = vkCreateDescriptorPool(_core.getTEngine()->getDevice(), &descriptorPoolInfo, nullptr, &_descriptorPool);
	assert(!vkRes);

}

void TGui::setupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(_descriptorPool, &_descriptorSetLayout, 1);

	VkResult vkRes = vkAllocateDescriptorSets(_core.getTEngine()->getDevice(), &allocInfo, &_descriptorSet);
	assert(!vkRes);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		//Binding 0 : for the VertexShader uniform buffer
		vkTools::initializers::writeDescriptorSet(
			_descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformDataVS.descriptor
		),
	};

	vkUpdateDescriptorSets(_core.getTEngine()->getDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

}

void TGui::buildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearvalue[2];

	clearvalue[0] = {};
}

void TGui::setUniformBuffers()
{
	uboVS.projection = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f);

	uint8_t *pData;
	VkResult err = vkMapMemory(_core.getTEngine()->getDevice(), uniformDataVS.memory, 0, sizeof(uboVS), 0, (void**)&pData);
	assert(!err);
	memcpy(pData, &uboVS, sizeof(uboVS));
	vkUnmapMemory(_core.getTEngine()->getDevice(), uniformDataVS.memory);

}

void TGui::destroyObjects()
{
	for (size_t i = 0; i < _objects.size(); i++) {
		delete _objects[i];
		_objects[i] = 0;
	}
}
