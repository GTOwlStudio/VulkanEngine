#include "TEngine.h"



TEngine::TEngine(bool enableValidation) : TEngineBase(enableValidation)
{
	/*width = 1280;
	height = 720;*/
	zoom = -2.5f;
	rotation = { 0.0f, 15.0f, 0.0f };
}


TEngine::~TEngine()
{

	vkDestroyImageView(device, texture.view, nullptr);
	vkDestroyImage(device, texture.image, nullptr);
	vkDestroySampler(device, texture.sampler, nullptr);
	vkFreeMemory(device, texture.deviceMemory, nullptr);

	vkDestroyPipeline(device, pipelines.solid, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, vertices.buf, nullptr);
	vkFreeMemory(device, vertices.mem, nullptr);

	vkDestroyBuffer(device, indices.buf, nullptr);
	vkFreeMemory(device, indices.mem, nullptr);
	
	/*vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
	vkDestroySemaphore(device, semaphores.renderComplete, nullptr);*/

	vkDestroyBuffer(device, uniformDataVS.buffer, nullptr);
	vkFreeMemory(device, uniformDataVS.memory, nullptr);
}

void TEngine::draw()
{
	TEngineBase::prepareFrame();

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	TEngineBase::submitFrame();
}

void TEngine::setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags ascpectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange)
{

	VkImageMemoryBarrier imageMemoryBarrier = vkTools::initializers::imageMemoryBarrier();
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	}

	switch (newImageLayout) 
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageFlags,
		dstStageFlags,
		VK_FLAGS_NONE,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

}

void TEngine::generateCoord(float x, float y, float w, float h, float depth)
{
#define DIM 1.0f
	std::vector<Vertex> vertexBuffer = {
		{ { x + w, y + h, depth },{ 1.0f, 1.0f } },
		{ { x, y + h, depth },{ 0.0f, 1.0f } },
		{ { x, y, depth },{ 0.0f, 0.0f } },
		{ { x + w, y, depth },{ 1.0f, 0.0f } }
		/*{ { DIM, DIM, 0.0f },{ 1.0f, 1.0f } },
		{ { -DIM, DIM, 0.0f },{ 0.0f, 1.0f } },
		{ { -DIM, -DIM, 0.0f },{ 0.0f, 0.0f } },
		{ { DIM, -DIM, 0.0f },{ 1.0f, 0.0f } }*/
	};
#undef DIM

	createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vertexBuffer.size() * sizeof(Vertex),
		vertexBuffer.data(),
		&vertices.buf,
		&vertices.mem);

	std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };
	indices.count = indexBuffer.size();

	createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		indexBuffer.size() * sizeof(uint32_t),
		indexBuffer.data(),
		&indices.buf,
		&indices.mem);

}

void TEngine::setupVertexDescription()
{
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] = vkTools::initializers::vertexInputBindingDescription(
		VERTEX_BUFFER_BIND_ID,
		sizeof(Vertex),
		VK_VERTEX_INPUT_RATE_VERTEX
	);
	//location 0 : coordinate
	vertices.attributeDescriptions.resize(2);
	vertices.attributeDescriptions[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);
	//location 1 : texture coordinate
	vertices.attributeDescriptions[1] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			sizeof(float) * 3);

	vertices.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();

}

void TEngine::prepareUniformBuffers()
{
	//Vertex Shader Uniform Buffer Block
	createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		sizeof(uboVS),
		&uboVS,
		&uniformDataVS.buffer,
		&uniformDataVS.memory,
		&uniformDataVS.descriptor);

	updateUniformBuffers();
}

void TEngine::updateUniformBuffers()
{
	//uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.001f, 256.0f);
	uboVS.projection = glm::ortho((float)0, (float)width, (float)0, (float)height);
	//glm::mat4 viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));
	glm::mat4 viewMatrix = glm::mat4(1.0);
	uboVS.model = viewMatrix * glm::translate(glm::mat4(), cameraPos);
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(1.0f, 0.0f, 1.0f));

	//uboVS.viewPos = glm::vec4(0.0, 0.0, -zoom, 0.0f);

	uint8_t *pData;
	VkResult err = vkMapMemory(device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void**)&pData);
	assert(!err);
	memcpy(pData, &uboVS, sizeof(uboVS));
	vkUnmapMemory(device, uniformDataVS.memory);

}

void TEngine::setupDescriptorSetLayout()
{

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		//Binding 0 for Vertex Shader uniform buffer
		vkTools::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT,
			0),
		//Binding 1 for Fragment Shader image sampler
		vkTools::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			1),

	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vkTools::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());


	VkResult err = vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &descriptorSetLayout);
	assert(!err);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vkTools::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);


	err = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	assert(!err);

}

void TEngine::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1),
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 2);

	VkResult vkRes = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
	assert(!vkRes);
}

void TEngine::setupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

	VkResult vkRes = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	assert(!vkRes);

	VkDescriptorImageInfo  texDescriptor =
		vkTools::initializers::descriptorImageInfo(texture.sampler, texture.view, VK_IMAGE_LAYOUT_GENERAL);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		//Binding 0 for Vertex Shader Uniform Buffer
		vkTools::initializers::writeDescriptorSet(
			descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformDataVS.descriptor),
		//Binding 1 for Fragment Shader texture Sampler
		vkTools::initializers::writeDescriptorSet(
			descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			&texDescriptor)
	};

	vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
}

void TEngine::loadTexture(std::string filename, VkFormat format, bool forceLinearTiling)
{
	gli::texture2D tex2D(gli::load(filename));
	assert(!tex2D.empty());

	VkFormatProperties formatProperties;

	texture.width = tex2D[0].dimensions().x;
	texture.height = tex2D[0].dimensions().y;
	texture.mipLevels = tex2D.levels();

	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	VkBool32 useStaging = true;

	if (forceLinearTiling) {
		useStaging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}

	VkMemoryAllocateInfo memAllocInfo = vkTools::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs = {};

	if (useStaging) {
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo();
		bufferCreateInfo.size = tex2D.size();

		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkTools::checkResult(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

		vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);

		vkTools::checkResult(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
		vkTools::checkResult(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

		uint8_t *data;
		vkTools::checkResult(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void**)&data));
		memcpy(data, tex2D.data(), tex2D.size());
		vkUnmapMemory(device, stagingMemory);

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < texture.mipLevels; i++) {
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = tex2D[i].dimensions().x;
			bufferCopyRegion.imageExtent.height = tex2D[i].dimensions().y;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += tex2D[i].size();
		}

		VkImageCreateInfo imageCreateInfo = vkTools::initializers::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = texture.mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageCreateInfo.extent = { texture.width, texture.height, 1 };
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture.image));


		vkGetImageMemoryRequirements(device, texture.image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAllocInfo.memoryTypeIndex);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture.deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device, texture.image, texture.deviceMemory, 0));

		VkCommandBuffer copyCmd = TEngineBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = texture.mipLevels;
		subresourceRange.layerCount = 1;

		setImageLayout(copyCmd, texture.image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_PREINITIALIZED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			texture.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			bufferCopyRegions.size(),
			bufferCopyRegions.data());

		texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		setImageLayout(copyCmd, texture.image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			texture.imageLayout,
			subresourceRange);

		TEngineBase::flushCommandBuffer(copyCmd, queue, true);


		vkFreeMemory(device, stagingMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);

	}

	else
	{
		//TODO
	}

	VkSamplerCreateInfo sampler = vkTools::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = (useStaging) ? (float)texture.mipLevels : 0.0f;
	sampler.maxAnisotropy = 8;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture.sampler));

	VkImageViewCreateInfo view = vkTools::initializers::imageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount = 1;

	view.subresourceRange.levelCount = (useStaging) ? texture.mipLevels : 1;
	view.image = texture.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture.view));
}


void TEngine::destroyTextureImage(Texture texture) {
	vkDestroyImage(device, texture.image, nullptr);
	vkFreeMemory(device, texture.deviceMemory, nullptr);
}

void TEngine::buildCommandBuffers() {
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValue[2];
	//clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };//defaultClearColor;
	clearValue[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };//defaultClearColor;
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	VkResult err;

	for (int32_t i = 0; i < drawCmdBuffers.size(); i++) {

		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vkTools::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
		//bind shader binding points
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

		//Bind triangle vertices
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertices.buf, offsets);
		//bind triangle vertices
		vkCmdBindIndexBuffer(drawCmdBuffers[i], indices.buf, 0, VK_INDEX_TYPE_UINT32);
		//draw indexed triangle
		vkCmdDrawIndexed(drawCmdBuffers[i], indices.count, 1, 0, 0, 0);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		err = vkEndCommandBuffer(drawCmdBuffers[i]);
		assert(!err);
	}


}


void TEngine::preparePipelines()
{


	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vkTools::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vkTools::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VkPipelineColorBlendStateCreateInfo   colorBlendState =
		vkTools::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState = vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), dynamicStateEnables.size(), 0);


	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = loadShader(getAssetPath() + "shaders/texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getAssetPath() + "shaders/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);

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

	VkResult err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid);
	assert(!err);

}


void TEngine::prepare()
{
	TEngineBase::prepare();
	generateCoord(10.0f, 10.0f, 10.0f, 10.0f, 0.0f);
	setupVertexDescription();
	prepareUniformBuffers();
	loadTexture(getAssetPath() + "textures/Fanatic_TriWave_1024.ktx", VK_FORMAT_BC2_UNORM_BLOCK, false);
	setupDescriptorSetLayout();
	preparePipelines();
	setupDescriptorPool();
	setupDescriptorSet();
	buildCommandBuffers();
	//gui.load();
	prepared = true;

}

void TEngine::render()
{
	if (!prepared) {
		return;
	}
	//gui.updateLogic();
	draw();
}


void TEngine::viewChanged()
{
	vkDeviceWaitIdle(device);
	updateUniformBuffers();
}


