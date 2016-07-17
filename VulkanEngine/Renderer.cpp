#include "Renderer.h"



CRenderer::CRenderer()
{
//	InitVulkan();
}


CRenderer::~CRenderer()
{

	vkDestroyBuffer(m_device, m_smem.buf, nullptr);
	vkFreeMemory(m_device, m_smem.mem, nullptr);

	m_swapChain.cleanup();
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

	vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);

	vkDestroyCommandPool(m_device, m_cmdPool, nullptr);

	vkDestroySemaphore(m_device, m_semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_device, m_semaphores.renderComplete, nullptr);

	vkDestroyDevice(m_device, nullptr);
	if (gEnv->enableValidation) {
		vkDebug::freeDebugCallback(m_instance);
	}
	vkDestroyInstance(m_instance, nullptr);

}

void CRenderer::Init()
{
	
	initSwapChain();

	if (gEnv->enableValidation) {
		vkDebug::setupDebugging(m_instance, VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT, NULL);
	}


	createCommandPool();
	createSetupCommandBuffer();
	setupSwapChain();
	createCommandBuffers();
	buildPresentCommandBuffers();
	setupDepthStencil();
	setupRenderPass();
	createPipelineCache();
	setupFrameBuffer();
	flushSetupCommandBuffer();
	createSetupCommandBuffer();

	createSBuffer(sizeof(uint32_t)*10, 0);

}

void CRenderer::render()
{
	draw();
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

	uint32_t graphicsQueueIndex = 0;
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

	VK_CHECK_RESULT(createDevice(queueCreateInfo, gEnv->enableValidation));

	// Store properties (including limits) and features of the phyiscal device
	// So examples can check against them and see if a feature is actually supported
	vkGetPhysicalDeviceProperties(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceFeatures);

	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice.physicalDevice, &m_physicalDevice.deviceMemoryProperties);

	vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &m_queue);

	VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(m_physicalDevice.physicalDevice, &m_depthFormat);
	assert(validDepthFormat);

	m_swapChain.connect(m_instance, m_physicalDevice.physicalDevice, m_device);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkTools::initializers::semaphoreCreateInfo();

	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.presentComplete));

	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.renderComplete));

	m_submitInfo = vkTools::initializers::submitInfo();
	m_submitInfo.pWaitDstStageMask = &submitPipelineStages;
	m_submitInfo.waitSemaphoreCount = 1;
	m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
	m_submitInfo.signalSemaphoreCount = 1;
	m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
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
	m_prePresentCmdBuffers.resize(m_swapChain.imageCount);
	m_postPresentCmdBuffers.resize(m_swapChain.imageCount);

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vkTools::initializers::commandBufferAllocateInfo(
			m_cmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(m_drawCmdBuffers.size()));

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_drawCmdBuffers.data()));

	// Command buffers for submitting present barriers
	// One pre and post present buffer per swap chain image
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_prePresentCmdBuffers.data()));
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_postPresentCmdBuffers.data()));
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
	vkTools::setImageLayout(
		m_setupCmdBuffer,
		m_depthStencil.image,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	depthStencilView.image = m_depthStencil.image;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &depthStencilView, nullptr, &m_depthStencil.view));
}
void CRenderer::setupRenderPass()
{
	VkAttachmentDescription attachments[2] = {};

	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = NULL;

	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}
void CRenderer::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(m_device, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
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
		vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_drawCmdBuffers.size()), m_prePresentCmdBuffers.data());
		vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_drawCmdBuffers.size()), m_postPresentCmdBuffers.data());
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

	submitFrame();
}

void CRenderer::prepareFrame()
{
	VK_CHECK_RESULT(m_swapChain.acquireNextImage(m_semaphores.presentComplete, &m_currentBuffer));

	VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_postPresentCmdBuffers[m_currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void CRenderer::submitFrame()
{
	VkSubmitInfo submitInfo = vkTools::initializers::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_prePresentCmdBuffers[m_currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));

	VK_CHECK_RESULT(m_swapChain.queuePresent(m_queue, m_currentBuffer, m_semaphores.renderComplete));

	VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));
}

void CRenderer::addGraphicPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo inputState, std::string name)
{
	pipelineCreateInfo.pVertexInputState = &inputState;
	m_pipelineName.push_back(name);
	m_pipelines.push_back(nullptr);
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.back()));
}

void CRenderer::buildCommandBuffer()
{
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValue[2];
	clearValue[0].color = { 0.025f, 0.025f, 0.025f, 1.0f};
	clearValue[0].depthStencil = {1.0f, 0};

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

		VkViewport viewport = vkTools::initializers::viewport(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i],0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

		vkCmdEndRenderPass(m_drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));

	}

}

void CRenderer::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
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
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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

VkResult CRenderer::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = gEnv->pSystem->getAppName().c_str();
	appInfo.pEngineName = gEnv->pSystem->getAppName().c_str();
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 11);

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

VkResult CRenderer::createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation)
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
}
