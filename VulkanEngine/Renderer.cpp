#include "Renderer.h"



CRenderer::CRenderer(PFN_GetEnabledFeatures enabledFeaturesFn) 
{

	if (enabledFeaturesFn != nullptr) {
		this->m_enabledFeatures = enabledFeaturesFn();
	}

//	InitVulkan();
}


CRenderer::~CRenderer()
{
	m_prepared = false;
	clean_dev();
	clearRessources();



	for (size_t i = 0; i < m_pipelines.pipelines.size();i++) {
		vkDestroyPipeline(m_device, m_pipelines.pipelines[i],nullptr);
	}

	dev_data.shader->clear(m_device);
	delete dev_data.shader;
	m_swapChain.cleanup();

	if (m_descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	}

	if (m_setupCmdBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_setupCmdBuffer);
	}
	destroyCommandBuffer();
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (uint32_t i = 0; i < m_frameBuffers.size();i++) {
		vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
	}

	vkDestroyImageView(m_device, m_depthStencil.view, nullptr);
	vkDestroyImage(m_device, m_depthStencil.image, nullptr);
	vkFreeMemory(m_device, m_depthStencil.mem, nullptr);

	vkDestroyPipelineCache(m_device, m_pipelines.pipelineCache, nullptr);

	vkDestroyCommandPool(m_device, m_cmdPool, nullptr);

	vkDestroySemaphore(m_device, m_semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_device, m_semaphores.renderComplete, nullptr);

	/*for (auto& fence : m_waitFences) {
		vkDestroyFence(m_vulkanDevice->logicalDevice, fence, nullptr);
	}*/

	delete m_vulkanDevice;

	if (gEnv->enableValidation) {
		vkDebug::freeDebugCallback(m_instance);
	}

	vkDestroyInstance(m_instance, nullptr);
	
}

void CRenderer::clearRessources()
{
	//Clean Textures
	for (size_t i = 0; i < m_textures.size(); i++) {
		m_textureLoader->destroyTexture(m_textures[i]);
	}

	if (m_textureLoader != nullptr) {
		delete(m_textureLoader);
	}

	//Clean Shaders
	for (size_t i = 0; i < m_shaders.shaders.size();i++) {
		m_shaders.shaders[i]->clear(m_device);
		delete m_shaders.shaders[i];
		m_shaders.shaders[i] = 0;
	}

	for (size_t i = 0; i < m_buffers.size();i++) {
		m_buffers[i].destroy();
	}
}

void CRenderer::Init()
{
	
	initSwapChain();

	if (gEnv->enableValidation) {
		vkDebug::setupDebugging(m_instance, VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, NULL);
	}

	if (m_vulkanDevice->enableDebugMarkers) {
		vkDebug::setupDebugMarker(m_device);
	}


	createCommandPool();
	createSetupCommandBuffer();
	
	setupSwapChain();
	createCommandBuffers();
//	buildPresentCommandBuffers();
	setupDepthStencil();
	setupRenderPass();
	createPipelineCache();
	setupFrameBuffer();
	flushSetupCommandBuffer();
	createSetupCommandBuffer();
	m_textureLoader = new vkTools::VulkanTextureLoader(m_vulkanDevice->physicalDevice, m_vulkanDevice->logicalDevice, m_queue, m_cmdPool);
	//createSBuffer(sizeof(uint32_t)*10, 0);

	//dev_test(100.0f, 100.0f, 100.0f, 100.0f, 0.1f);
	setupDescriptorPool();



	dev_test(-0.5f, -0.5f, 1.0f, 1.0f, 0.1f);
	
	/*dev_prepareUBO();
	dev_setupDescriptorSet();
	*/
	buildCommandBuffer();

	m_prepared = true;

}

void CRenderer::InitVulkan()
{
	VkResult err;

	err = createInstance();

	if (err) {
		vkTools::exitFatal("Cound not create Vulkan instance : \n" + vkTools::errorString(err), "Fatal error");
	}

	uint32_t gpuCount = 0;

	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	assert(gpuCount>0);

	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
	if (err)
	{
		vkTools::exitFatal("Could not enumerate physical devices : \n" + vkTools::errorString(err), "Fatal error");
	}

	m_physicalDevice.physicalDevice = physicalDevices[0];

	m_vulkanDevice = new vk::VulkanDevice(m_physicalDevice.physicalDevice);
	VK_CHECK_RESULT(m_vulkanDevice->createLogicalDevice(m_enabledFeatures));
	m_device = m_vulkanDevice->logicalDevice;

	/*uint32_t graphicsQueueIndex = 0;
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice.physicalDevice, &queueCount, NULL);
	assert(queueCount >= 1);

	std::vector<VkQueueFamilyProperties> queueProps;
	queueProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice.physicalDevice, &queueCount, queueProps.data());

	for (graphicsQueueIndex = 0; graphicsQueueIndex<queueCount; graphicsQueueIndex++)
	{
		if (queueProps[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			break;
		}
	}
	assert(graphicsQueueIndex<queueCount);

	std::array<float, 1> queuePriorities = { 0.0f };
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	//VK_CHECK_RESULT(createDevice(queueCreateInfo, gEnv->enableValidation));
	*/
	// Store properties (including limits) and features of the phyiscal device
	// So examples can check against them and see if a feature is actually supported
	vkGetPhysicalDeviceProperties(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceFeatures);

	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceMemoryProperties);

	vkGetDeviceQueue(m_device, m_vulkanDevice->queueFamilyIndices.graphics, 0, &m_queue);

	VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(m_physicalDevice.physicalDevice, &m_depthFormat);
	assert(validDepthFormat);

	m_swapChain.connect(m_instance, m_physicalDevice.physicalDevice, m_device);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkTools::initializers::semaphoreCreateInfo();

	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.presentComplete));

	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.renderComplete));

	//VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	m_submitInfo = vkTools::initializers::submitInfo();
	m_submitInfo.pWaitDstStageMask = &submitPipelineStages;
	m_submitInfo.waitSemaphoreCount = 1;
	m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
	m_submitInfo.signalSemaphoreCount = 1;
	m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
}

void CRenderer::render()
{

	if (!m_prepared) {
		return;
	}
	draw();
}

vk::VulkanDevice * CRenderer::getVulkanDevice()
{
	return m_vulkanDevice;
}

vkTools::VulkanTextureLoader * CRenderer::getTextureLoader()
{
	return m_textureLoader;
}

vkTools::CShader * CRenderer::getShader(std::string shaderName)
{
	for (size_t i = 0; i < m_shaders.shaders.size();i++) {
		if (shaderName==m_shaders.names[i]) {
			return m_shaders.shaders[i];
		}
	}

	printf("Shader %s does not exit\n", shaderName.c_str());

	return nullptr;
}

VkBuffer CRenderer::getBuffer(uint32_t id)
{
	if (id>=m_buffers.size()) {
		printf("ERROR : id %i out of range, buffer.size()=%i\n", id, m_buffers.size());
		return nullptr;
	}
	if (m_buffers[id].buffer==nullptr){
		printf("ERROR : there is no buffer at id %\n", id);
		return nullptr;
	}
	return m_buffers[id].buffer;
}

VkBool32 CRenderer::getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1 == 1)) {
			if ((m_physicalDevice.deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				*typeIndex = i;
				return true;
			}
		}
	}
	return false;
}

uint32_t CRenderer::getMemoryType(uint32_t typeBits, VkFlags properties)
{
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			if ((m_physicalDevice.deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return 1;
			}
			typeBits >>= 1;
		}
	}
	return 0;
}

VkPipelineShaderStageCreateInfo CRenderer::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = vkTools::loadShader(fileName.c_str(), m_device, stage);
	shaderStage.pName = "main";
	assert(shaderStage.module != NULL);
	dev_data.shaderModules.push_back(shaderStage.module);

	return shaderStage;
}

void CRenderer::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = m_swapChain.queueNodeIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool));
}
void CRenderer::createCommandBuffers()
{
	m_drawCmdBuffers.resize(m_swapChain.imageCount);
	/*m_prePresentCmdBuffers.resize(m_swapChain.imageCount);
	m_postPresentCmdBuffers.resize(m_swapChain.imageCount);*/

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vkTools::initializers::commandBufferAllocateInfo(
			m_cmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(m_drawCmdBuffers.size()));

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_drawCmdBuffers.data()));

	// Command buffers for submitting present barriers
	// One pre and post present buffer per swap chain image
	/*VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_prePresentCmdBuffers.data()));
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_postPresentCmdBuffers.data()));*/
}
void CRenderer::setupDepthStencil()
{

	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.pNext = NULL;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = m_depthFormat;
	image.extent = { gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.pNext = NULL;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = m_depthFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(m_device, &image, nullptr, &m_depthStencil.image));
	vkGetImageMemoryRequirements(m_device, m_depthStencil.image, &memReqs);
	mem_alloc.allocationSize = memReqs.size;
	mem_alloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &mem_alloc, nullptr, &m_depthStencil.mem));

	VK_CHECK_RESULT(vkBindImageMemory(m_device, m_depthStencil.image, m_depthStencil.mem, 0));
	
/*	vkTools::setImageLayout(
		m_setupCmdBuffer,
		m_depthStencil.image,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);*/

	depthStencilView.image = m_depthStencil.image;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &depthStencilView, nullptr, &m_depthStencil.view));
}
void CRenderer::setupRenderPass()
{
	std::array<VkAttachmentDescription,2> attachments = {};

	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	/*attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;*/

	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}
void CRenderer::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(m_device, &pipelineCacheCreateInfo, nullptr, &m_pipelines.pipelineCache));
}
void CRenderer::setupFrameBuffer()
{
	VkImageView attachments[2];
	attachments[1] = m_depthStencil.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = m_renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = gEnv->pSystem->getWidth();
	frameBufferCreateInfo.height = gEnv->pSystem->getHeight();
	frameBufferCreateInfo.layers = 1;

	m_frameBuffers.resize(m_swapChain.imageCount);
	for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
		attachments[0] = m_swapChain.buffers[i].view;
		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
	}
}
void CRenderer::createSetupCommandBuffer()
{
	if (m_setupCmdBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_setupCmdBuffer);
		m_setupCmdBuffer = VK_NULL_HANDLE;
	}

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vkTools::initializers::commandBufferAllocateInfo(m_cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_setupCmdBuffer));

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_RESULT(vkBeginCommandBuffer(m_setupCmdBuffer, &cmdBufInfo));
}

void CRenderer::flushSetupCommandBuffer()
{

	if (m_setupCmdBuffer == VK_NULL_HANDLE) {
		return;
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(m_setupCmdBuffer));

	VkSubmitInfo submitInfo_l = {};
	submitInfo_l.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo_l.commandBufferCount = 1;
	submitInfo_l.pCommandBuffers = &m_setupCmdBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo_l, VK_NULL_HANDLE));

	VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));

	vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_setupCmdBuffer);
	m_setupCmdBuffer = VK_NULL_HANDLE;
}

/*
void CRenderer::buildPresentCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	for (uint32_t i = 0; i < m_swapChain.imageCount; i++) {


		//Command buffer for post presentBarrier
		// Insert a post present image barrier to transform the image back to a
		// color attachment that our render pass can write to
		// We always use undefined image layout as the source as it doesn't actually matter
		// what is done with the previous image contents

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_postPresentCmdBuffers[i], &cmdBufInfo));

		VkImageMemoryBarrier postPresentBarrier = vkTools::initializers::imageMemoryBarrier();
		postPresentBarrier.srcAccessMask = 0;
		postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		postPresentBarrier.image = m_swapChain.buffers[i].image;

		vkCmdPipelineBarrier(
			m_postPresentCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &postPresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_postPresentCmdBuffers[i]));

		//Command buffers for pre present barrier

		// Submit a pre present image barrier to the queue
		// Transforms the (framebuffer) image layout from color attachment to present(khr) for presenting to the swap chain

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_prePresentCmdBuffers[i], &cmdBufInfo));

		VkImageMemoryBarrier prePresentBarrier = vkTools::initializers::imageMemoryBarrier();
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		prePresentBarrier.image = m_swapChain.buffers[i].image;

		vkCmdPipelineBarrier(
			m_prePresentCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &prePresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_prePresentCmdBuffers[i]));
	}

}
*/

void CRenderer::initSwapChain()
{
	m_swapChain.initSurface( gEnv->pSystem->getWindowInstance(), gEnv->pSystem->getWindow());
}

void CRenderer::setupSwapChain()
{
	m_swapChain.create(m_setupCmdBuffer, gEnv->pSystem->getWidthPtr(), gEnv->pSystem->getHeightPtr());
}

void CRenderer::destroyCommandBuffer()
{
		vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_drawCmdBuffers.size()), m_drawCmdBuffers.data());
		/*vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_prePresentCmdBuffers.size()), m_prePresentCmdBuffers.data());
		vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_postPresentCmdBuffers.size()), m_postPresentCmdBuffers.data());*/
}

bool CRenderer::checkCommandBuffers()
{
	for (auto& cmdBuffer :m_drawCmdBuffers) {
		if (cmdBuffer == VK_NULL_HANDLE) {
			return false;
		}
	}
	return true;
}

VkCommandBuffer CRenderer::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vkTools::initializers::commandBufferAllocateInfo(m_cmdPool, level, 1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &cmdBuffer));
	if (begin) {
		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
	}
	
	return cmdBuffer; 
}

void CRenderer::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
{
	if (commandBuffer == VK_NULL_HANDLE) {
		return;
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(queue));

	if (free) {
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &commandBuffer);
	}

}

void CRenderer::draw()
{
	prepareFrame();

	m_submitInfo.commandBufferCount = 1;
	m_submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer];

	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));

	/*VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]));*/

	submitFrame();


}

void CRenderer::prepareFrame()
{
	VK_CHECK_RESULT(m_swapChain.acquireNextImage(m_semaphores.presentComplete, &m_currentBuffer));

	/*VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_postPresentCmdBuffers[m_currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));*/
}

void CRenderer::submitFrame()
{
/*	VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_prePresentCmdBuffers[m_currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));*/

	VK_CHECK_RESULT(m_swapChain.queuePresent(m_queue, m_currentBuffer, m_semaphores.renderComplete));

	VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));
}

void CRenderer::addGraphicsPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo const& inputState, std::string name)
{
	pipelineCreateInfo.pVertexInputState = &inputState;
	m_pipelines.pipelineNames.push_back(name);
	m_pipelines.pipelines.push_back(nullptr);
	
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelines.pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.pipelines.back()));
}

void CRenderer::addGraphicsPipeline(VkPipelineLayout pipelineLayout ,VkRenderPass renderPass, VkPipelineCreateFlags flags, VkPrimitiveTopology topology, VkPolygonMode polyMode, uint32_t shaderStagesCount, VkPipelineShaderStageCreateInfo * shaderStages, VkPipelineVertexInputStateCreateInfo const & inputState, std::string name)
{
	m_pipelines.pipelinesState.push_back({});
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = 
		vkTools::initializers::pipelineCreateInfo(&m_pipelines.pipelinesState.back(), pipelineLayout, renderPass, flags, topology, polyMode, shaderStagesCount, shaderStages);
	
	pipelineCreateInfo.pVertexInputState = &inputState;
	m_pipelines.pipelineNames.push_back(name);
	m_pipelines.pipelines.push_back(nullptr);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelines.pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.pipelines.back()));
}

void CRenderer::addShader(std::string vsPath, std::string fsPath, std::string * shaderName, std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings, std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription)
{
	//Checking if the name is not already use
	for (size_t i = 0;i<m_shaders.shaders.size();i++){
		if (*shaderName==m_shaders.names[i]) {
			printf("WARNING : addShader, shaderName %s already used, it as been replaced by %s%i", m_shaders.names[i], m_shaders.names[i], (int)i);
			*shaderName += std::to_string(i);
		}
	}

	m_shaders.names.push_back(*shaderName);
	m_shaders.shaders.push_back(new vkTools::CShader(vsPath, fsPath, setLayoutBindings, bindingDescription, attributeDescription));
}

void CRenderer::addWriteDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets)
{
	for (size_t i = 0; i < writeDescriptorSets.size();i++) {
		writeDescriptorSets.push_back(writeDescriptorSets[i]);
	}
}

void CRenderer::updateDescriptorSets()
{
	for (size_t i = 0; i < m_writeDescriptorSets.size();i++) {
		vkUpdateDescriptorSets(m_device, (uint32_t)m_writeDescriptorSets.size(), m_writeDescriptorSets.data(), 0,nullptr);
	}
}

void CRenderer::addIndexedDraw(SIndexedDrawInfo drawInfo)
{
	m_indexedDraws.push_back(drawInfo);
}

void CRenderer::buildDrawCommands()
{
	if (!checkCommandBuffers()) {
		destroyCommandBuffer();
		createCommandBuffers();
	}
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValue[2];
	clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
	renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	for (int32_t i = 0; i < m_drawCmdBuffers.size();i++) {
		renderPassBeginInfo.framebuffer = m_frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

		for (int32_t j = 0; j < m_indexedDraws.size();j++) {
			vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_indexedDraws[j].pipelineLayout, 
				0, 1, m_indexedDraws[j].descriptorSets, 0, nullptr);
		
			vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_indexedDraws[j].pipeline);
			
			vkCmdBindVertexBuffers(m_drawCmdBuffers[i], 0, 1, m_indexedDraws[j].vertexBuffer, m_indexedDraws[j].pVertexOffset);
			
			vkCmdBindIndexBuffer(m_drawCmdBuffers[i], 
				*m_indexedDraws[j].vertexBuffer, 
				m_indexedDraws[j].indexOffset, 
				m_indexedDraws[j].indexType);
			
			vkCmdDrawIndexed(m_drawCmdBuffers[i], 
				m_indexedDraws[j].indexCount, 
				m_indexedDraws[j].indexCount, 
				m_indexedDraws[j].firstIndex, 
				m_indexedDraws[j].vertexOffset, 
				m_indexedDraws[j].firstInstance);
		}
		
		vkCmdEndRenderPass(m_drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));

	}

}

void CRenderer::initRessources()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
	};

	std::vector<VkVertexInputBindingDescription> bindings = 
	{
		vkTools::initializers::vertexInputBindingDescription(0, sizeof(VertexT), VK_VERTEX_INPUT_RATE_VERTEX)
	};

	std::vector<VkVertexInputAttributeDescription> attributes =
	{
		vkTools::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, 2 * sizeof(float))
	};
	
	std::string shaderName = "texture";

	addShader(gEnv->getAssetpath()+"shaders/texture.vert.spv", gEnv->getAssetpath() + "shaders/texture.frag.spv",
		&shaderName,setLayoutBindings, bindings, attributes);


}

void CRenderer::handleMessages(WPARAM wParam, LPARAM lParam)
{
	/*dev_data.rotationZ += 20.0f;
	dev_updateUniform_2();
	printf("here");*/

	switch (wParam) {
	case 0x50:
		//printf("here");
		dev_data.rotationZ += 1.0f;
		dev_updateUniform_2();
		break;
	case 0x42: //B
		printf("%s\n",buffersLayoutToString().c_str());
		break;
	}
}

void CRenderer::createBuffer(uint32_t * id, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size)
{
	m_buffers.push_back({});
	if (id!=nullptr){
		*id = m_buffers.size();
	}
	m_buffers.back().device = m_device;
	m_buffers.back().usageFlags = usageFlags;
	m_buffers.back().memoryPropertyFlags = memoryFlags;
	m_buffers.back().size = size;

	createBuffer(m_buffers.back().usageFlags, m_buffers.back().memoryPropertyFlags, size, 0, &m_buffers.back().buffer, &m_buffers.back().memory);
}

void CRenderer::bufferSubData(uint32_t id, VkDeviceSize size, VkDeviceSize offset, void * data)
{
	writeInBuffer(&m_buffers[id-1].buffer, size, data, offset);
}

void CRenderer::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = 
	{
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo((uint32_t)poolSizes.size(), poolSizes.data(), 1);

	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));

}

void CRenderer::buildCommandBuffer()
{
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValue[2];
	clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f};
	clearValue[1].depthStencil = {1.0f, 0};

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
	renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	
	for (int32_t i = 0; i < m_drawCmdBuffers.size();i++) {
		renderPassBeginInfo.framebuffer = m_frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i],0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

		//vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, dev_data.pipelineLayout, 0, 1, &dev_data.descriptorSet, 0, NULL);
		vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, dev_data.shader->getPipelineLayout(), 0, 1, &dev_data.descriptorSet, 0, NULL);
		vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.pipelines.back());

		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(m_drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &m_smem.buf, offsets);

		vkCmdBindIndexBuffer(m_drawCmdBuffers[i], m_smem.buf, dev_data.vertices.size() * sizeof(Vertex), VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(m_drawCmdBuffers[i], (uint32_t)dev_data.indices.size(), 1, 0, 0, 0);
		
		vkCmdEndRenderPass(m_drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));

	}

}

VkBool32 CRenderer::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
{
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
	VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(usageFlags, size);

	VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, buffer));

	vkGetBufferMemoryRequirements(m_device, *buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &memAlloc.memoryTypeIndex);
	VK_CHECK_RESULT(vkAllocateMemory(m_device,&memAlloc, nullptr, memory));
	if (data!=nullptr) {
		void* mapped;
		VK_CHECK_RESULT(vkMapMemory(m_device, *memory, 0, size, 0, &mapped));
		memcpy(mapped, data, size);
		vkUnmapMemory(m_device, *memory);
	}
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, *buffer, *memory, 0));
	return true;
}

VkBool32 CRenderer::createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
{
	return createBuffer(usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, size, data, buffer, memory);
}

VkBool32 CRenderer::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory, VkDescriptorBufferInfo * descriptor)
{
	VkBool32 res = createBuffer(usage, memFlags, size, data, buffer, memory);
	if (res) {
		descriptor->offset = 0;
		descriptor->buffer = *buffer;
		descriptor->range = size;
		return true;
	}
	else {
		return false;
	}
}

void CRenderer::createTexture(uint32_t * id, VkImageCreateInfo imageCreateInfo, uint8_t *datas, uint32_t width, uint32_t height)
{
	vkTools::VulkanTexture tex = {};
	tex.width = width;
	tex.height = height;
	//tex.imageLayout = 
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo allocInfo = vkTools::initializers::memoryAllocateInfo();

	if (m_vulkanDevice->enableDebugMarkers) {
		vkDebug::setObjectName(m_vulkanDevice->logicalDevice, (uint64_t)tex.image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "texture");
	}

	VK_CHECK_RESULT(vkCreateImage(m_vulkanDevice->logicalDevice, &imageCreateInfo, nullptr, &tex.image));

	vkGetImageMemoryRequirements(m_vulkanDevice->logicalDevice, tex.image, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = m_vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(m_vulkanDevice->logicalDevice, &allocInfo, nullptr, &tex.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(m_vulkanDevice->logicalDevice, tex.image, tex.deviceMemory, 0));

	struct {
		VkDeviceMemory mem;
		VkBuffer buf;
	} stagingBuffer;

	VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo();
	bufferCreateInfo.size = allocInfo.allocationSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	VK_CHECK_RESULT(vkCreateBuffer(m_vulkanDevice->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer.buf));

	vkGetBufferMemoryRequirements(m_vulkanDevice->logicalDevice, stagingBuffer.buf, &memReqs);

	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = m_vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(m_vulkanDevice->logicalDevice, &allocInfo, nullptr, &stagingBuffer.mem));
	VK_CHECK_RESULT(vkBindBufferMemory(m_vulkanDevice->logicalDevice, stagingBuffer.buf, stagingBuffer.mem, 0));

	uint8_t *data;
	VK_CHECK_RESULT(vkMapMemory(m_vulkanDevice->logicalDevice, stagingBuffer.mem, 0, allocInfo.allocationSize, 0, (void**)&data));
	memcpy(data, datas, width*height);
	vkUnmapMemory(m_vulkanDevice->logicalDevice, stagingBuffer.mem);

	VkCommandBuffer copyCmd = {};

	copyCmd = m_vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	vkTools::setImageLayout(copyCmd, tex.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = width;
	bufferCopyRegion.imageExtent.height = height;
	bufferCopyRegion.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(copyCmd, stagingBuffer.buf, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	vkTools::setImageLayout(copyCmd, tex.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	flushCommandBuffer(copyCmd, m_queue, true);

	vkFreeMemory(m_vulkanDevice->logicalDevice, stagingBuffer.mem, nullptr);
	vkDestroyBuffer(m_vulkanDevice->logicalDevice, stagingBuffer.buf, nullptr);
	
	VkImageViewCreateInfo imageViewInfo = vkTools::initializers::imageViewCreateInfo();
	imageViewInfo.image = tex.image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = imageCreateInfo.format;
	imageViewInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
	imageViewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

	VK_CHECK_RESULT(vkCreateImageView(m_vulkanDevice->logicalDevice, &imageViewInfo, nullptr, &tex.view));

	VkSamplerCreateInfo samplerInfo = vkTools::initializers::samplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(m_vulkanDevice->logicalDevice, &samplerInfo, nullptr, &tex.sampler));

	m_textures.push_back(tex);
	*id = m_textures.size();
}

void CRenderer::createSBuffer(VkDeviceSize size, void* data)
{
	/*VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
	VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);

	vkTools::checkResult(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_smem.buf));*/

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		size,
		data,
		&stagingBuffer,
		&stagingMemory);

	createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		size,
		nullptr,
		&m_smem.buf,
		&m_smem.mem);

	VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(
		copyCmd,
		stagingBuffer,
		m_smem.buf,
		1,
		&copyRegion);

	flushCommandBuffer(copyCmd, m_queue, true);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingMemory, nullptr);

}

void CRenderer::writeInBuffer(VkBuffer * dstBuffer, VkDeviceSize size, void * data, VkDeviceSize dstOffset)
{
	//vkDeviceWaitIdle(m_device);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	
	createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		size,
		data,
		&stagingBuffer,
		&stagingMemory);

	VkCommandBuffer copyCmd  = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	copyRegion.dstOffset = dstOffset;

	vkCmdCopyBuffer(
		copyCmd,
		stagingBuffer,
		*dstBuffer,
		1,
		&copyRegion);

	flushCommandBuffer(copyCmd, m_queue, true);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingMemory, nullptr);
	
}

void CRenderer::dev_test(float x, float y, float w, float h, float depth)
{


	std::vector<Vertex> tmpV = {
	{ { x + w, y + h, depth },{ 1.0f, 1.0f, 0.0f} },
	{ { x, y + h, depth },{ 0.0f, 1.0f, 0.0f} },
	{ { x, y, depth },{ 0.0f, 1.0f, 1.0f} },
	{ { x + w, y, depth },{ 1.0f, 0.0f, 1.0f } } };
	
	dev_data.vertices.resize(4);

	for (size_t i = 0; i < tmpV.size();i++) {
		dev_data.vertices[i] = tmpV[i];
	}

	std::vector<uint32_t> tmpI = {0,1,2,	2,3,0};
	
	dev_data.indices.resize(tmpI.size());

	for (size_t i = 0; i < tmpI.size();i++) {
		dev_data.indices[i] = tmpI[i];
	}
	
	VkDeviceSize vsize = tmpV.size() * sizeof(Vertex);
	VkDeviceSize isize = tmpI.size() * sizeof(uint32_t);
	VkDeviceSize uboSize = sizeof(dev_data.uboVS);

	
	//dev_updateUniform_2();
	createSBuffer(vsize+isize+uboSize, dev_data.vertices.data()); //create sbuffer and copy vertex(pos and color) to it
	writeInBuffer(&m_smem.buf, isize,dev_data.indices.data(), vsize); //Copy index data to sbuffer
	//writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, vsize+isize);

	
	

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT,
		0)
	};

	/*VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vkTools::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &dev_data.descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vkTools::initializers::pipelineLayoutCreateInfo(&dev_data.descriptorSetLayout, 1);

	vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo,nullptr, &dev_data.pipelineLayout);*/



	/*std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = loadShader("./data/shaders/basic.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader("./data/shaders/basic.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	*/
	

	dev_data.bindingDescriptions.resize(1);
	dev_data.bindingDescriptions[0] =
		vkTools::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			sizeof(Vertex),
			VK_VERTEX_INPUT_RATE_VERTEX
		);

	dev_data.attributeDescriptions.resize(2);
	dev_data.attributeDescriptions[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);
	dev_data.attributeDescriptions[1] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float)*3
		);
		
	/*dev_data.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	dev_data.inputState.vertexBindingDescriptionCount = dev_data.bindingDescriptions.size();
	dev_data.inputState.pVertexBindingDescriptions = dev_data.bindingDescriptions.data();
	dev_data.inputState.vertexAttributeDescriptionCount = dev_data.attributeDescriptions.size();
	dev_data.inputState.pVertexAttributeDescriptions = dev_data.attributeDescriptions.data();*/

	dev_data.shader = new vkTools::CShader("./data/shaders/basic.vert.spv", "./data/shaders/basic.frag.spv", setLayoutBindings, dev_data.bindingDescriptions, dev_data.attributeDescriptions);
	dev_data.shader->load(m_device);
	//addGraphicPipeline(pipelineCreateInfo, dev_data.inputState, "devp");

	m_pipelines.pipelinesState.push_back({});

	/*VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(
			&m_pipelinesState.back(),
			dev_data.pipelineLayout,
			m_renderPass,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			2,
			shaderStages.data());*/

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = dev_data.shader->getShaderStages();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(
			&m_pipelines.pipelinesState.back(),
			dev_data.shader->getPipelineLayout(),
			m_renderPass,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			2,
			//shaderStages.data());
			dev_data.shader->getShaderStagesPtr());
	
	//std::cout << &dev_data.shader->getShaderStages() << std::endl;

	addGraphicsPipeline(pipelineCreateInfo, dev_data.shader->getInputState(), "devp");
	
	writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, vsize + isize);
	
	dev_updateUniform_2();
	
	
	dev_data.uniformDataVS.descriptor.offset = vsize + isize;
	dev_data.uniformDataVS.descriptor.range = sizeof(dev_data.uboVS);
	dev_data.uniformDataVS.descriptor.buffer = m_smem.buf;

	dev_setupDescriptorSet();
	//dev_prepareUBO();
	

	//dev_setupDescriptorSet();
	//dev_prepareUBO();
	

}

void CRenderer::dev_test2(float x, float y, float w, float h, float depth)
{


	std::vector<Vertex> tmpV = {
		{ { x + w, y + h, depth },{ 1.0f, 1.0f, 0.0f } },
		{ { x, y + h, depth },{ 0.0f, 1.0f, 0.0f } },
		{ { x, y, depth },{ 0.0f, 1.0f, 1.0f } },
		{ { x + w, y, depth },{ 1.0f, 0.0f, 1.0f } } };

	dev_data.vertices.resize(4);

	for (size_t i = 0; i < tmpV.size(); i++) {
		dev_data.vertices[i] = tmpV[i];
	}

	std::vector<uint32_t> tmpI = { 0,1,2,	2,3,0 };

	dev_data.indices.resize(tmpI.size());

	for (size_t i = 0; i < tmpI.size(); i++) {
		dev_data.indices[i] = tmpI[i];
	}

	VkDeviceSize vsize = tmpV.size() * sizeof(Vertex);
	VkDeviceSize isize = tmpI.size() * sizeof(uint32_t);
	VkDeviceSize uboSize = sizeof(dev_data.uboVS);


	//dev_updateUniform_2();
	createSBuffer(vsize + isize + uboSize, dev_data.vertices.data()); //create sbuffer and copy vertex(pos and color) to it
	writeInBuffer(&m_smem.buf, isize, dev_data.indices.data(), vsize); //Copy index data to sbuffer
																	   //writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, vsize+isize);




	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT,
		0)
	};

	/*VkDescriptorSetLayoutCreateInfo descriptorLayout =
	vkTools::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &dev_data.descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
	vkTools::initializers::pipelineLayoutCreateInfo(&dev_data.descriptorSetLayout, 1);

	vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo,nullptr, &dev_data.pipelineLayout);*/



	/*std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = loadShader("./data/shaders/basic.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader("./data/shaders/basic.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	*/


	dev_data.bindingDescriptions.resize(1);
	dev_data.bindingDescriptions[0] =
		vkTools::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			sizeof(Vertex),
			VK_VERTEX_INPUT_RATE_VERTEX
		);

	dev_data.attributeDescriptions.resize(2);
	dev_data.attributeDescriptions[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0);
	dev_data.attributeDescriptions[1] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(float) * 3
		);

	/*dev_data.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	dev_data.inputState.vertexBindingDescriptionCount = dev_data.bindingDescriptions.size();
	dev_data.inputState.pVertexBindingDescriptions = dev_data.bindingDescriptions.data();
	dev_data.inputState.vertexAttributeDescriptionCount = dev_data.attributeDescriptions.size();
	dev_data.inputState.pVertexAttributeDescriptions = dev_data.attributeDescriptions.data();*/

	dev_data.shader = new vkTools::CShader("./data/shaders/basic.vert.spv", "./data/shaders/basic.frag.spv", setLayoutBindings, dev_data.bindingDescriptions, dev_data.attributeDescriptions);
	dev_data.shader->load(m_device);
	//addGraphicPipeline(pipelineCreateInfo, dev_data.inputState, "devp");

	m_pipelines.pipelinesState.push_back({});

	/*VkGraphicsPipelineCreateInfo pipelineCreateInfo =
	vkTools::initializers::pipelineCreateInfo(
	&m_pipelinesState.back(),
	dev_data.pipelineLayout,
	m_renderPass,
	0,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	VK_POLYGON_MODE_FILL,
	2,
	shaderStages.data());*/

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = dev_data.shader->getShaderStages();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(
			&m_pipelines.pipelinesState.back(),
			dev_data.shader->getPipelineLayout(),
			m_renderPass,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			2,
			//shaderStages.data());
			dev_data.shader->getShaderStagesPtr());

	//std::cout << &dev_data.shader->getShaderStages() << std::endl;

	addGraphicsPipeline(pipelineCreateInfo, dev_data.shader->getInputState(), "devp");

	writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, vsize + isize);

	dev_updateUniform_2();


	dev_data.uniformDataVS.descriptor.offset = vsize + isize;
	dev_data.uniformDataVS.descriptor.range = sizeof(dev_data.uboVS);
	dev_data.uniformDataVS.descriptor.buffer = m_smem.buf;

	dev_setupDescriptorSet();
	//dev_prepareUBO();


	//dev_setupDescriptorSet();
	//dev_prepareUBO();


}

void CRenderer::dev_setupDescriptorSet()
{
	/*VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(m_descriptorPool, &dev_data.descriptorSetLayout, 1);*/

	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(m_descriptorPool, dev_data.shader->getDescriptorSetLayoutPtr(), 1);

	VkResult vkRes = vkAllocateDescriptorSets(m_device, &allocInfo, &dev_data.descriptorSet);
	assert(!vkRes);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		//Binding 0 for Vertex Shader Uniform Buffer
		vkTools::initializers::writeDescriptorSet(
			dev_data.descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&dev_data.uniformDataVS.descriptor),

	};

	vkUpdateDescriptorSets(m_device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
}

void CRenderer::dev_prepareUBO()
{
	createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(dev_data.uboVS),
		nullptr,
		&dev_data.uniformDataVS.buffer,
		&dev_data.uniformDataVS.memory,
		&dev_data.uniformDataVS.descriptor);
	dev_updateUniform();
}

void CRenderer::dev_updateUniform() //v1
{
	dev_data.uboVS.projection = glm::perspective(glm::radians(60.0f), (float)gEnv->pSystem->getWidth()/(float)gEnv->pSystem->getHeight(), 0.1f, 256.0f);
	
	//dev_data.uboVS.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight(), 0.1f, 100.0f);

	dev_data.uboVS.view = glm::lookAt(glm::vec3(1.0f,1.0f,1.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_device, dev_data.uniformDataVS.memory, 0, sizeof(dev_data.uboVS), 0, (void**)&pData));
	memcpy(pData, &dev_data.uboVS, sizeof(dev_data.uboVS));
	vkUnmapMemory(m_device, dev_data.uniformDataVS.memory);

}

void CRenderer::dev_updateUniform_2() //v0
{
	dev_data.uboVS.projection = glm::perspective(glm::radians(60.0f), (float)gEnv->pSystem->getWidth() / (float)gEnv->pSystem->getHeight(), 0.1f, 256.0f);



	//dev_data.uboVS.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight(), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(dev_data.rotationZ), glm::vec3(0.0f, 0.0f, 1.0f));

	dev_data.uboVS.view = view;// *  glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	
	//dev_data.uboVS.view = dev_data

	writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, (dev_data.vertices.size()*sizeof(Vertex))+(dev_data.indices.size()*sizeof(uint32_t)));

/*	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_device, m_smem.mem,( sizeof(uint32_t)* dev_data.indices.size()) + (sizeof(Vertex) * dev_data.vertices.size()), sizeof(dev_data.uboVS), 0, (void**)&pData));
	memcpy(pData, &dev_data.uboVS, sizeof(dev_data.uboVS));
	vkUnmapMemory(m_device, m_smem.mem);*/

}

void CRenderer::clean_dev()
{

	vkDestroyBuffer(m_device, m_smem.buf, nullptr);
	vkFreeMemory(m_device, m_smem.mem, nullptr);
	vkDestroyPipelineLayout(m_device, dev_data.pipelineLayout, nullptr);
}

VkResult CRenderer::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = gEnv->pSystem->getAppName().c_str();
	appInfo.pEngineName = gEnv->pSystem->getAppName().c_str();
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if defined (_WIN32)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif 

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (enabledExtensions.size() > 0)
	{
		if (gEnv->enableValidation)
		{
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	if (gEnv->enableValidation)
	{
		instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
		instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
	}

	return vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
}

std::string CRenderer::buffersLayoutToString()
{
	std::string s = "";
	s += std::to_string(m_buffers.size());
	s += "\n";
	for (size_t i = 0; i < m_buffers.size();i++) {
		s += "Buffer " + std::to_string(i);
		s += "\nsize=" + std::to_string(m_buffers[i].size);
	}
	return s;
}

/*VkResult CRenderer::createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation)
{

	std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &requestedQueues;
	deviceCreateInfo.pEnabledFeatures = NULL;

	if (enabledExtensions.size()>0) {
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	if (enableValidation) {
		deviceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
		deviceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
	}

	return vkCreateDevice(m_physicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_device);
}*/
