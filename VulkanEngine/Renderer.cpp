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
		//vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &m_shaders.descriptorSets[i]); //#enhancement avoid the use of iteration
		if (m_shaders.descriptorSets[i]!=nullptr) {
			vkFreeDescriptorSets(m_device, m_shaders.descriptorPool, 1, &m_shaders.descriptorSets[i]); //#enhancement avoid the use of iteration
		}
		

	}
	
	//Destroy descriptorSets

	/*for (size_t i = 0; i < m_shaders.descriptorSets.size(); i++) {
		delete m_shaders.descriptorSets[i];
		m_shaders.descriptorSets[i] = 0;
	}*/

	//dev_data.shader->clear(m_device);
	delete dev_data.shader;
	m_swapChain.cleanup();

	if (m_shaders.descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, m_shaders.descriptorPool, nullptr);
	}
	/*
	if (m_descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	}
	*/
	if (m_setupCmdBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_setupCmdBuffer);
	}
	if (m_offscreenCmdBuffer != VK_NULL_HANDLE) {
		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_offscreenCmdBuffer);
	}
	destroyCommandBuffer();
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (uint32_t i = 0; i < m_framebuffers.size();i++) {
		vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
	}

	vkDestroyImageView(m_device, m_depthStencil.view, nullptr);
	vkDestroyImage(m_device, m_depthStencil.image, nullptr);
	vkFreeMemory(m_device, m_depthStencil.mem, nullptr);

	vkDestroyPipelineCache(m_device, m_pipelines.pipelineCache, nullptr);

	/*vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
	m_cmdPool = VK_NULL_HANDLE;*/

	vkDestroySemaphore(m_device, m_semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_device, m_semaphores.renderComplete, nullptr);

	/*for (auto& fence : m_waitFences) {
		vkDestroyFence(m_vulkanDevice->logicalDevice, fence, nullptr);
	}*/
	vkDeviceWaitIdle(m_vulkanDevice->logicalDevice);
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
	



	//RenderPasses
	for (VkRenderPass rp : m_renderPasses.renderPasses) {
		vkDestroyRenderPass(m_device, rp, nullptr);
	}

	for (size_t i = 0; i < m_buffers.size();i++) {
		m_buffers[i].destroy();
	}

	//delete m_dfb;
	for (size_t i = 0; i < m_offscreenTargets.size();i++) {
		if (m_offscreenTargets[i]!=nullptr) {
			delete m_offscreenTargets[i];
			m_offscreenTargets[i] = 0;
		}
	}
}

void CRenderer::Init()
{
	
	initSwapChain();

	if (gEnv->enableValidation) {
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT;// | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		vkDebug::setupDebugging(m_instance, debugReportFlags, VK_NULL_HANDLE);
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
	//setupDescriptorPool();

	initRessources();
	//printf("%i\n", &m_shaders.descriptorSets[0]);
//	dev_test(-0.5f, -0.5f, 1.0f, 1.0f, 0.1f);
	//printf("%i\n", &m_shaders.descriptorSets[0]);
	//loadShader();
	/*dev_prepareUBO();
	dev_setupDescriptorSet();
	*/
	//buildCommandBuffer();

	//printf("%i\n", &m_shaders.descriptorSets[0]);
	//m_prepared = true;

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
VkRenderPass CRenderer::getRenderPass(std::string renderPassName)
{
	for (size_t i = 0; i < m_renderPasses.renderPasses.size();i++) {
		if (renderPassName==m_renderPasses.names[i]) {
			return m_renderPasses.renderPasses[i];
		}
	}
	return VK_NULL_HANDLE;
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
vkTools::CShader * CRenderer::getShader(uint32_t id)
{
	printf("ERROR : YOU SHOULDN'T HAVE THIS MESSAGE DISPLAYING, it means that you use getShader(uint32_t id), the function isn't yet implemented");
	return nullptr;
}
uint32_t CRenderer::getShaderId(std::string shaderName)
{
	for (size_t i = 0; i < m_shaders.shaders.size();i++) {
		if (shaderName == m_shaders.names[i]) {
			return static_cast<uint32_t>(i);
		}
	}
	return UINT32_MAX;
}
size_t CRenderer::getShaderLastBinding()
{
	return m_shaders.shaders.size();
}

uint64_t CRenderer::getBufferAvaibleId()
{
	uint64_t id = UINT64_MAX;

	for (size_t i = 0; i < m_buffers.size();i++) {
		if (m_buffers[i].buffer==nullptr) {
			id = i;
			break;
		}
	}

	if (id==UINT64_MAX) {
		id = m_buffers.size() - 1;
	}

	return id;
}

VkBuffer CRenderer::getBuffer(uint64_t id)
{
	if (id>m_buffers.size()) {
		printf("ERROR : id %" PRId64 "out of range, buffer.size()=%i\n", id, static_cast<int>(m_buffers.size()));
		return VK_NULL_HANDLE;
	}
	if (m_buffers[id].buffer==VK_NULL_HANDLE){
		printf("ERROR : there is no buffer at id %" PRId64 "\n", id);
		return VK_NULL_HANDLE;
	}
	return m_buffers[id].buffer;
}

vk::Buffer* CRenderer::getBufferStruct(uint32_t id)
{
	if (id>m_buffers.size()) {
		printf("ERROR : id %i out of range, buffer.size()=%i", id, static_cast<int>(m_buffers.size()));
		return nullptr;
	}
	if (m_buffers[id-1].buffer==VK_NULL_HANDLE) {
		printf("ERROR : There is no buffer at id %i\n", id);
		return nullptr;
	}
	return &m_buffers[id - 1];
}

vkTools::VulkanTexture* CRenderer::getTexture(uint32_t texId)
{
	if (texId>m_textures.size()) {
		return nullptr;
	}
	if (&m_textures[texId-1]==nullptr) {
		return nullptr;
	}
	return &m_textures[texId-1];
}

VkDescriptorSet * CRenderer::getDescriptorSet(uint32_t id)
{
	if (id>=m_shaders.descriptorSets.size()) {
		printf("The id %i isn't in the array\n", id);
		return nullptr;
	}
	if (m_shaders.descriptorSets[id]==VK_NULL_HANDLE) 
	{
		printf("The descriptor at %i doesn't exist\n", id);
		return nullptr;
	}
	return &m_shaders.descriptorSets[id];
}

VkPipeline CRenderer::getPipeline(std::string pipelineName)
{
	for (size_t i = 0; i < m_pipelines.pipelineNames.size();i++) {
		if (m_pipelines.pipelineNames[i]==pipelineName) {
			return m_pipelines.pipelines[i];
		}
	}
	printf("ERROR : pipeline %s seems top not exist\n", pipelineName.c_str());
	return VK_NULL_HANDLE;
}

VkDescriptorPool CRenderer::getDescriptorPool(uint32_t id)
{
	return m_shaders.descriptorPool;
}

VkCommandPool CRenderer::getCommandPool()
{
	return m_cmdPool;
}

CFramebuffer* CRenderer::getOffscreen(std::string name)
{
	return helper::iterate<CFramebuffer*>(name, m_offscreenTargets,m_offscreenNames);
}

uint32_t CRenderer::requestDescriptorSet(VkDescriptorType type, uint32_t descriptorCount, std::string descriptorLayoutName)
{
	m_shaders.poolSize.push_back(vkTools::initializers::descriptorPoolSize(type, 1));
	m_shaders.descriptorSets.push_back({});
	//m_shaders.descriptorTypes.push_back(type);
	m_shaders.descriptorLayoutNames.push_back(descriptorLayoutName);
	//return m_shaders.descriptorSets.back();
	return static_cast<uint32_t>(m_shaders.descriptorSets.size()-1);
}

uint32_t CRenderer::requestDescriptorSet(std::vector<VkDescriptorType> types, uint32_t descriptorCount, std::string descriptorLayoutName)
{
	for (size_t i = 0; i < types.size();i++) {
		m_shaders.poolSize.push_back(vkTools::initializers::descriptorPoolSize(types[i], 1));
	}
	m_shaders.descriptorSets.push_back({});
	//m_shaders.descriptorTypes.push_back(type);
	m_shaders.descriptorLayoutNames.push_back(descriptorLayoutName);
	//return m_shaders.descriptorSets.back();
	return static_cast<uint32_t>(m_shaders.descriptorSets.size() - 1);
}

void CRenderer::getInfo()
{
	printf("-------Renderer Info-------\n\nPIPELINES\n");
	printf("pipelines.count = %i\npipelinesNames.count = %i\npipelineStates.count = %i\nPipeline Names : \n", static_cast<int>(m_pipelines.pipelines.size()), static_cast<int>(m_pipelines.pipelineNames.size()), static_cast<int>(m_pipelines.pipelinesState.size()));
	for (size_t i = 0; i < m_pipelines.pipelineNames.size();i++) 
	{
		printf("\t\"%s\"\n", m_pipelines.pipelineNames[i].c_str());
	}
	
	printf("\nSHADERS\n");
	printf("shaders.count = %i\nshadersNames.count = %i\ndescriptorPool.count = %i\ndescriptorSets.count = %i\nShader Names : \n", static_cast<int>(m_shaders.shaders.size()), static_cast<int>(m_shaders.names.size()),1, static_cast<int>(m_shaders.descriptorSets.size()));
	for (size_t i = 0; i < m_shaders.names.size();i++) {
		printf("\t\"%s\"\n", m_shaders.names[i].c_str());
	}

	printf("\nRENDER PASSES\n");
	printf("renderPasses.count = %i\nrenderPassesNames.count = %i\nRenderpasses Name\n", static_cast<int>(m_renderPasses.renderPasses.size()), static_cast<int>(m_renderPasses.names.size()));
	for (std::string s : m_renderPasses.names) {
		printf("\t\"%s\"\n", s.c_str());
	}
	printf("\nOffscreen Target");
	printf("offscreenTargets.count = %i\noffscreenNames.count =%i\nOffscreenTargetsName\n", static_cast<int>(m_offscreenTargets.size()), static_cast<int>(m_offscreenNames.size()));
	for (std::string s : m_offscreenNames) {
		printf("\t\"%s\"\n", s.c_str());
	}
	printf("\nDrawList");
	printf("drawList.size() = %i\n", m_drawsList.size());
	for (size_t i = 0; i < m_drawsList.size();i++) {
		printf("\t(%i) %s\n", m_drawsList[i], m_drawsListTags[i].c_str());
	}
	
	printf("\n");
}

void CRenderer::getBufferInfo() 
{
	for (size_t i = 0; i < m_buffers.size();i++) {
		printf("%s\n",helper::flagsToString(m_buffers[i].usageFlags, " ").c_str());
	}
}

void CRenderer::bcb()
{
	//buildCommandBuffer();
	//buildDrawCommands(m_renderPass);
	//buildDrawCommands(getRenderPass("offscreen"));
	//buildDrawCommands(getRenderPass("main"));
//	m_offscreen = false; //BIG WARNING
	graphicsInit();
	if (m_offscreen) {
		buildOffscreenDrawCommands();
	}
	buildTargetedDrawCommands();
	buildDrawCommands ();
	
}

void CRenderer::graphicsInit()
{
	VkRenderPass renderPass[2];
	createRenderPass(&renderPass[0], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR, true, true);
	createRenderPass(&renderPass[1], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_LOAD, true, false);
	//VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &renderPass));

	VkCommandBuffer cmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	VkClearValue clearValue[2];
	clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
	renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	for (uint32_t i = 0; i < m_swapChain.imageCount; i++) {
		renderPassBeginInfo.renderPass = renderPass[i];
		renderPassBeginInfo.framebuffer = m_framebuffers[i];
		//clearValue[1].depthStencil = { 0.9f, 0 };

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		/*VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);*/
		vkCmdEndRenderPass(cmdBuffer);
	}
	//Swapchain images clearing

	/*for (size_t i = 0; i < m_swapChain.imageCount;i++) {
		vkCmdClearColorImage(cmdBuffer, m_swapChain.imageCount[i], VK_IMAGE_LAYOUT_UNDEFINED, );
	}*/

	flushCommandBuffer(cmdBuffer, m_queue, true);
	vkDestroyRenderPass(m_device, renderPass[0], nullptr);
	vkDestroyRenderPass(m_device, renderPass[1], nullptr);
}

VkBool32 CRenderer::getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {//#MCE operator priorities
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
	m_cmdPool = m_vulkanDevice->commandPool;
	//m_cmdPool = m_vulkanDevice->createCommandPool(m_swapChain.queueNodeIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	/*VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = m_swapChain.queueNodeIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool));*/

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

	m_renderPasses.attachementDescriptions = attachments;
	m_renderPasses.colorReference = colorReference;
	m_renderPasses.depthReference = depthReference;
	m_renderPasses.subpassDescription = subpassDescription;
	m_renderPasses.subpassDependencies = dependencies;

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
	
	//m_framebuffers.resize(m_swapChain.imageCount);

	for (uint32_t i = 0; i < m_swapChain.imageCount; i++) {
		attachments[0] = m_swapChain.buffers[i].view;
		addFramebuffer(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), m_renderPass, 2, attachments);
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
	if (m_offscreen) {

		m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
		m_submitInfo.pSignalSemaphores = &m_offscreenSemaphore;
		
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &m_offscreenCmdBuffer;

		VK_CHECK_RESULT(vkQueueSubmit(m_queue,1,&m_submitInfo,VK_NULL_HANDLE));

		m_submitInfo.pWaitSemaphores = &m_offscreenSemaphore;
		m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
		
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer];

		VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));
	}
	else {
		m_submitInfo.commandBufferCount = 1;
		m_submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer];

		VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));
	}
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
	m_pipelines.pipelines.push_back(VK_NULL_HANDLE);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelines.pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.pipelines.back()));
}

void CRenderer::addGraphicsPipeline(VkPipelineLayout pipelineLayout ,VkRenderPass renderPass, VkPipelineCreateFlags flags, VkPrimitiveTopology topology, VkPolygonMode polyMode, 
	uint32_t shaderStagesCount, VkPipelineShaderStageCreateInfo * shaderStages, VkPipelineVertexInputStateCreateInfo const & inputState, std::string name)
{
	m_pipelines.pipelinesState.push_back({});
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = 
		vkTools::initializers::pipelineCreateInfo(&m_pipelines.pipelinesState.back(), pipelineLayout, renderPass, flags, topology, polyMode, shaderStagesCount, shaderStages);
	
	pipelineCreateInfo.pVertexInputState = &inputState;
	m_pipelines.pipelineNames.push_back(name);
	m_pipelines.pipelines.push_back(VK_NULL_HANDLE);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelines.pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.pipelines.back()));
}

void CRenderer::addGraphicsPipeline(vkTools::CShader * shader, VkRenderPass renderPass, std::string name,bool blend, VkPipelineCreateFlags flags, VkPrimitiveTopology topology, VkPolygonMode polyMode, uint32_t shaderStagesCount)
{
	if (blend==false) {
		addGraphicsPipeline(shader->getPipelineLayout(), renderPass, flags, topology, polyMode, shaderStagesCount, shader->getShaderStagesPtr(), shader->getInputState(), name);

	}
	else {
		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vkTools::initializers::pipelineColorBlendAttachmentState(0xf, VK_TRUE);

		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vkTools::initializers::pipelineColorBlendStateCreateInfo(1,&blendAttachmentState);

		m_pipelines.pipelinesState.push_back({});
		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vkTools::initializers::pipelineCreateInfo(&m_pipelines.pipelinesState.back(), shader->getPipelineLayout(), renderPass, flags, topology, polyMode, shaderStagesCount, shader->getShaderStagesPtr());
		pipelineCreateInfo.pColorBlendState = &colorBlendState;

		pipelineCreateInfo.pVertexInputState = &shader->getInputState();
		m_pipelines.pipelineNames.push_back(name);
		m_pipelines.pipelines.push_back(VK_NULL_HANDLE);

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelines.pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.pipelines.back()));

	}
}

void CRenderer::addRenderPass(std::string renderPassName, VkAttachmentLoadOp loadOp)
{

	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	// Don't clear the framebuffer (like the renderpass from the example does)
	//attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].loadOp = loadOp;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (m_renderPasses.renderPasses.size() > 1) {
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else {
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//#warning
	}
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment
	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (m_renderPasses.renderPasses.size() > 1) {
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	else {
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//#warning
	}
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Use subpass dependencies for image layout transitions
	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	
	/*subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;*/

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;

	m_renderPasses.renderPasses.push_back(VK_NULL_HANDLE);
	m_renderPasses.names.push_back(renderPassName);

	vkCreateRenderPass(m_vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &m_renderPasses.renderPasses.back());
}

void CRenderer::addRenderPass(std::string renderPassName, VkAttachmentDescription colorAttachmentDescription)
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	// Don't clear the framebuffer (like the renderpass from the example does)
	//attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].loadOp = colorAttachmentDescription.loadOp;
	attachments[0].storeOp = colorAttachmentDescription.storeOp;
	attachments[0].stencilLoadOp = colorAttachmentDescription.stencilLoadOp;
	attachments[0].stencilStoreOp = colorAttachmentDescription.stencilStoreOp;
	attachments[0].initialLayout = colorAttachmentDescription.initialLayout;
	attachments[0].finalLayout = colorAttachmentDescription.finalLayout;

	// Depth attachment
	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Use subpass dependencies for image layout transitions
	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;

	/*subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;*/

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;

	m_renderPasses.renderPasses.push_back(VK_NULL_HANDLE);
	m_renderPasses.names.push_back(renderPassName);

	vkCreateRenderPass(m_vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &m_renderPasses.renderPasses.back());
}

void CRenderer::addShader(std::string vsPath, std::string fsPath, std::string * shaderName, std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings, std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription)
{
	//Checking if the name is not already use
	for (size_t i = 0;i<m_shaders.shaders.size();i++){
		if (*shaderName==m_shaders.names[i]) {
			printf("WARNING : addShader, shaderName %s already used, it as been replaced by %s%i", m_shaders.names[i].c_str(), m_shaders.names[i].c_str(), (int)i);
			*shaderName += std::to_string(i);
		}
	}



	m_shaders.names.push_back(*shaderName);
	m_shaders.shaders.push_back(new vkTools::CShader(vsPath, fsPath, setLayoutBindings, bindingDescription, attributeDescription));
	/*m_shaders.descriptorSets.push_back({});

	/*VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(m_descriptorPool, m_shaders.shaders.back()->getDescriptorSetLayoutPtr(), 1);*/

/*	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(m_shaders.descriptorPool, m_shaders.shaders.back()->getDescriptorSetLayoutPtr(), 1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_shaders.descriptorSets.back()));*/


}

VkDescriptorSet CRenderer::addDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout * pDescriptorLayout, uint32_t descriptorLayoutCount)
{
	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(descriptorPool, pDescriptorLayout, descriptorLayoutCount);

	m_shaders.descriptorSets.push_back({});
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_shaders.descriptorSets.back()));
	return m_shaders.descriptorSets.back();
}

void CRenderer::createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout * pDescriptorLayout, uint32_t descriptorLayoutCount, VkDescriptorSet * dstDescriptor)
{
	VkDescriptorSetAllocateInfo allocInfo = vkTools::initializers::descriptorSetAllocateInfo(descriptorPool, pDescriptorLayout, 1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, dstDescriptor));
}

void CRenderer::createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout * pDescriptorLayout, uint32_t descriptorLayoutCount, uint32_t id)
{
	VkDescriptorSetAllocateInfo allocInfo = vkTools::initializers::descriptorSetAllocateInfo(descriptorPool, pDescriptorLayout, 1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_shaders.descriptorSets[id]));

}

void CRenderer::addWriteDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets)
{
	for (size_t i = 0; i < writeDescriptorSets.size();i++) {
		m_writeDescriptorSets.push_back(writeDescriptorSets[i]);
		/*m_writeDescriptorSets.push_back(new VkWriteDescriptorSet);
		*m_writeDescriptorSets.back() = writeDescriptorSets[i];*/
	}
}

void CRenderer::addCopyDescriptorSet(std::vector<VkCopyDescriptorSet> copyDescriptorSets)
{
	for (size_t i = 0; i < copyDescriptorSets.size(); i++) {
		m_copyDescriptorSets.push_back(copyDescriptorSets[i]);
	}
}

VkFramebuffer CRenderer::addFramebuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, uint32_t attachmentCount, VkImageView *pAttachments)
{
	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = renderPass;
	fbufCreateInfo.attachmentCount = attachmentCount;
	fbufCreateInfo.pAttachments = pAttachments;
	fbufCreateInfo.width = width;
	fbufCreateInfo.height = height;
	fbufCreateInfo.layers = 1;
	m_framebuffers.push_back(VK_NULL_HANDLE);
	
	vkCreateFramebuffer(m_device, &fbufCreateInfo, nullptr, &m_framebuffers.back());
	if (m_vulkanDevice->enableDebugMarkers) {
		vkDebug::setObjectName(m_vulkanDevice->logicalDevice, (uint64_t)m_framebuffers.back(), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "texture" + (m_textures.size() - 1));
	}
	return m_framebuffers.back();
}

CFramebuffer* CRenderer::addOffscreen(std::string name)
{
	if (helper::nameUsed(name, m_offscreenNames)) {
		return nullptr;
	}
	m_offscreenTargets.push_back(new CFramebuffer());
	m_offscreenNames.push_back(name);
	return m_offscreenTargets.back();
//	return CFramebuffer();
}

void CRenderer::updateDescriptorSets()
{
	vkUpdateDescriptorSets(m_device, (uint32_t)m_writeDescriptorSets.size(), m_writeDescriptorSets.data(), (uint32_t)m_copyDescriptorSets.size(), m_copyDescriptorSets.data());

	//vkUpdateDescriptorSets(m_device, (uint32_t)m_writeDescriptorSets.size(), m_writeDescriptorSets[0], 0, nullptr);
	
	/*for (size_t i = 0; i < m_writeDescriptorSets.size();i++) {
		vkUpdateDescriptorSets(m_device, (uint32_t)m_writeDescriptorSets.size(), *m_writeDescriptorSets.data(), 0,nullptr);
		printf("address %i\n", &m_writeDescriptorSets[0]);
	}*/
}

void CRenderer::loadShaders()
{
	for (size_t i = 0; i < m_shaders.shaders.size();i++) {
		m_shaders.shaders[i]->load(m_device);
	}
}

void CRenderer::addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::string tag)
{
	m_indexedDraws.push_back(drawInfo);
	m_renderAttachments.renderPasses.push_back(renderPass);
	m_renderAttachments.framebuffers.push_back(m_framebuffers[0]);
	m_renderAttachments.framebuffers.push_back(m_framebuffers[1]);
	m_renderAttachments.framebufferOffsets.push_back(2);
	m_renderAttachments.isOffscreen.push_back(false);

	if (tag!="NO") {
		addDrawToDrawList(m_indexedDraws.size()-1,tag);
	}

}

void CRenderer::addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::vector<VkFramebuffer> framebuffers)
{
	m_indexedDraws.push_back(drawInfo);
	m_renderAttachments.renderPasses.push_back(renderPass);
	m_renderAttachments.framebufferOffsets.push_back(static_cast<uint32_t>(framebuffers.size()));
	for (uint32_t i = 0; i < framebuffers.size();i++) {
		m_renderAttachments.framebuffers.push_back(framebuffers[i]);
	}
}

void CRenderer::addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, VkFramebuffer framebuffer)
{
	m_indexedDraws.push_back(drawInfo);
	m_renderAttachments.renderPasses.push_back(renderPass);
	m_renderAttachments.framebuffers.push_back(framebuffer);
	m_renderAttachments.framebufferOffsets.push_back(1);
	m_renderAttachments.isOffscreen.push_back(true);
	if (m_offscreen==false) {
		m_offscreenCmdBuffer = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
	}
	m_offscreen = true;

}

void CRenderer::addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::string targetName)
{
	if (!helper::nameUsed(targetName, m_offscreenInfos.names)) {
		m_offscreenInfos.targets.push_back(COffscreenTarget());
		m_offscreenInfos.targets.back().load(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), renderPass);
		m_offscreenInfos.names.push_back(targetName);
	}

	m_offscreenAttachments.draws.push_back(drawInfo);
	m_offscreenAttachments.targetNames.push_back(targetName);
	m_offscreenAttachments.cmdBuffers.push_back(createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false));
}

void CRenderer::addDrawToDrawList(uint64_t drawId, std::string tag)
{
	if (drawId>=m_indexedDraws.size()) {
		return;
	}
	m_drawsList.push_back(drawId);
	if (tag == "none") {
		m_drawsListTags.push_back(std::to_string(drawId));
	}
	else {
		m_drawsListTags.push_back(tag);
	}
}

void CRenderer::swap(uint64_t ida, uint64_t idb)
{
	if ((ida >= m_drawsList.size()) || (idb >= m_drawsList.size())) {
		printf("Oulal\n");
		return;
	}
	uint64_t tmp = m_drawsList[ida];
	m_drawsList[ida] = m_drawsList[idb];
	m_drawsList[idb] = tmp;
	std::string tmps = m_drawsListTags[ida];
	m_drawsListTags[ida] = m_drawsListTags[idb];
	m_drawsListTags[idb] = tmps;
}

void CRenderer::buildDrawCommands(VkRenderPass renderPass)
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
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
	renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	for (int32_t i = 0; i < m_drawCmdBuffers.size();i++) {
		renderPassBeginInfo.framebuffer = m_framebuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

		for (int32_t j = 0; j < m_indexedDraws.size();j++) {
			vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[j].pipelineLayout, 
				0, 1, m_indexedDraws[j].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets
		
			vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[j].pipeline);
			
			vkCmdBindVertexBuffers(m_drawCmdBuffers[i], m_indexedDraws[j].firstBinding, m_indexedDraws[j].bindingCount, &m_indexedDraws[j].vertexBuffer, m_indexedDraws[j].pVertexOffset);
			
			vkCmdBindIndexBuffer(m_drawCmdBuffers[i], 
				m_indexedDraws[j].indexBuffer, 
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

void CRenderer::buildDrawCommands()
{

	if (!checkCommandBuffers()) {
		destroyCommandBuffer();
		createCommandBuffers();
	}

	//for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r+=m_renderAttachments.framebufferOffsets[r]) {
	for (int32_t i = 0; i < m_drawCmdBuffers.size(); i++) {
		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();
		VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));
		for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r++) {
			if (m_renderAttachments.renderPasses[r] == VK_NULL_HANDLE) {
				continue;
			}
			if (m_renderAttachments.isOffscreen[r] == true) { //If the draw is offscreen it's not proccesed here
				continue;
			}
			

			VkClearValue clearValue[2];
			clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
			clearValue[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
			renderPassBeginInfo.renderPass = m_renderAttachments.renderPasses[r];
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
			renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues = clearValue;
			std::cout << "Coucou" << std::endl;
			
		
				renderPassBeginInfo.framebuffer = m_framebuffers[i];

				vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
				vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

				VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
				vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

				vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipelineLayout,
					0, 1, m_indexedDraws[r].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets

				vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipeline);

				vkCmdBindVertexBuffers(m_drawCmdBuffers[i], m_indexedDraws[r].firstBinding, m_indexedDraws[r].bindingCount, &m_indexedDraws[r].vertexBuffer, m_indexedDraws[r].pVertexOffset);

				vkCmdBindIndexBuffer(m_drawCmdBuffers[i],
					m_indexedDraws[r].indexBuffer,
					m_indexedDraws[r].indexOffset,
					m_indexedDraws[r].indexType);

				/*vkCmdDrawIndexed(m_drawCmdBuffers[i],
					m_indexedDraws[r].indexCount,
					m_indexedDraws[r].indexCount,
					m_indexedDraws[r].firstIndex,
					m_indexedDraws[r].vertexOffset,
					m_indexedDraws[r].firstInstance);*/

				vkCmdDrawIndexed(m_drawCmdBuffers[i],
					m_indexedDraws[r].indexCount,
					1,
					m_indexedDraws[r].firstIndex,
					m_indexedDraws[r].vertexOffset,
					m_indexedDraws[r].firstInstance);
				//	}

				vkCmdEndRenderPass(m_drawCmdBuffers[i]);
			

			}
		VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));
	}

}

void CRenderer::buildDrawCommands_old()
{

	if (!checkCommandBuffers()) {
		destroyCommandBuffer();
		createCommandBuffers();
	}

	//for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r+=m_renderAttachments.framebufferOffsets[r]) {
	for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r++) {
		if (m_renderAttachments.renderPasses[r] == VK_NULL_HANDLE) {
			continue;
		}
		if (m_renderAttachments.isOffscreen[r] == true) { //If the draw is offscreen it's not proccesed here
			continue;
		}
		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

		VkClearValue clearValue[2];
		clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
		clearValue[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_renderAttachments.renderPasses[r];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
		renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValue;
		std::cout << "Coucou" << std::endl;

		for (int32_t i = 0; i < m_drawCmdBuffers.size(); i++) {
			renderPassBeginInfo.framebuffer = m_framebuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
			vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
			vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipelineLayout,
				0, 1, m_indexedDraws[r].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets

			vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipeline);

			vkCmdBindVertexBuffers(m_drawCmdBuffers[i], m_indexedDraws[r].firstBinding, m_indexedDraws[r].bindingCount, &m_indexedDraws[r].vertexBuffer, m_indexedDraws[r].pVertexOffset);

			vkCmdBindIndexBuffer(m_drawCmdBuffers[i],
				m_indexedDraws[r].indexBuffer,
				m_indexedDraws[r].indexOffset,
				m_indexedDraws[r].indexType);

			vkCmdDrawIndexed(m_drawCmdBuffers[i],
				m_indexedDraws[r].indexCount,
				m_indexedDraws[r].indexCount,
				m_indexedDraws[r].firstIndex,
				m_indexedDraws[r].vertexOffset,
				m_indexedDraws[r].firstInstance);
			//	}

			vkCmdEndRenderPass(m_drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));

		}
	}

}

void CRenderer::buildOffscreenDrawCommands()
{
	if (!checkCommandBuffers()) {
		destroyCommandBuffer();
		createCommandBuffers();
	}

	//for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r+=m_renderAttachments.framebufferOffsets[r]) {
	for (uint32_t r = 0; r < m_renderAttachments.renderPasses.size(); r ++) {
		if (m_renderAttachments.renderPasses[r] == VK_NULL_HANDLE) {
			continue;
		}

		if (m_renderAttachments.isOffscreen[r]==false) {
			continue;
		}

		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

		VkClearValue clearValue[2];
		clearValue[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValue[1].depthStencil = { 1.0f, 0 };
		//std::cout << std::unitbuf;
		VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_renderAttachments.renderPasses[r];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
		renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValue;
		
		for (int32_t i = 0; i < 1; i++) { //#enhancement = flexibility
			renderPassBeginInfo.framebuffer = m_renderAttachments.framebuffers[getRenderAttachementFramebufferOffset(r)];
			//renderPassBeginInfo.framebuffer = m_framebuffers[2];
			//renderPassBeginInfo.framebuffer = m_framebuffers[0];

			VK_CHECK_RESULT(vkBeginCommandBuffer(m_offscreenCmdBuffer, &cmdBufInfo));

			vkCmdBeginRenderPass(m_offscreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
			vkCmdSetViewport(m_offscreenCmdBuffer, 0, 1, &viewport);

			VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
			vkCmdSetScissor(m_offscreenCmdBuffer, 0, 1, &scissor);

			/*for (int32_t j = 0; j < m_indexedDraws.size(); j++) {
				vkCmdBindDescriptorSets(m_offscreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[j].pipelineLayout,
					0, 1, m_indexedDraws[j].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets

				vkCmdBindPipeline(m_offscreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[j].pipeline);

				vkCmdBindVertexBuffers(m_offscreenCmdBuffer, 0, 1, &m_indexedDraws[j].vertexBuffer, m_indexedDraws[j].pVertexOffset);

				vkCmdBindIndexBuffer(m_offscreenCmdBuffer,
					m_indexedDraws[j].indexBuffer,
					m_indexedDraws[j].indexOffset,
					m_indexedDraws[j].indexType);

				vkCmdDrawIndexed(m_offscreenCmdBuffer,
					m_indexedDraws[j].indexCount,
					m_indexedDraws[j].indexCount,
					m_indexedDraws[j].firstIndex,
					m_indexedDraws[j].vertexOffset,
					m_indexedDraws[j].firstInstance);
			}*/

			//for (int32_t j = 0; j < m_indexedDraws.size(); j++) {
				vkCmdBindDescriptorSets(m_offscreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipelineLayout,
					0, 1, m_indexedDraws[r].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets

				vkCmdBindPipeline(m_offscreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_indexedDraws[r].pipeline);

				vkCmdBindVertexBuffers(m_offscreenCmdBuffer, m_indexedDraws[r].firstBinding, m_indexedDraws[r].bindingCount, &m_indexedDraws[r].vertexBuffer, m_indexedDraws[r].pVertexOffset);

				vkCmdBindIndexBuffer(m_offscreenCmdBuffer,
					m_indexedDraws[r].indexBuffer,
					m_indexedDraws[r].indexOffset,
					m_indexedDraws[r].indexType);

				vkCmdDrawIndexed(m_offscreenCmdBuffer,
					m_indexedDraws[r].indexCount,
					m_indexedDraws[r].indexCount,
					m_indexedDraws[r].firstIndex,
					m_indexedDraws[r].vertexOffset,
					m_indexedDraws[r].firstInstance);
			//}

			vkCmdEndRenderPass(m_offscreenCmdBuffer);

			VK_CHECK_RESULT(vkEndCommandBuffer(m_offscreenCmdBuffer));

		}
	}
}

void CRenderer::buildTargetedDrawCommands()
{
	uint64_t id;
	for (size_t i = 0; i < m_offscreenAttachments.targetNames.size();i++) {
		id = getOffscreenTargetId(m_offscreenAttachments.targetNames[i]);

		VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();
		VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_renderAttachments.renderPasses[i];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_offscreenInfos.targets[i].getWidth();
		renderPassBeginInfo.renderArea.extent.height = m_offscreenInfos.targets[i].getHeight();
		renderPassBeginInfo.clearValueCount = 0;
		renderPassBeginInfo.pClearValues = nullptr;
		renderPassBeginInfo.framebuffer = m_offscreenInfos.targets[i].getFrameBuffer();
		VK_CHECK_RESULT(vkBeginCommandBuffer(m_offscreenAttachments.cmdBuffers[i], &cmdBufInfo));
		
		vkCmdBeginRenderPass(m_offscreenAttachments.cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


		VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_offscreenAttachments.cmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_offscreenAttachments.cmdBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(m_offscreenAttachments.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenAttachments.draws[i].pipelineLayout,
			0, 1, m_offscreenAttachments.draws[i].descriptorSets, 0, nullptr); //#enhancement allowed multiple descriptor sets

		vkCmdBindPipeline(m_offscreenAttachments.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_offscreenAttachments.draws[i].pipeline);

		vkCmdBindVertexBuffers(m_offscreenAttachments.cmdBuffers[i], m_offscreenAttachments.draws[i].firstBinding, m_offscreenAttachments.draws[i].bindingCount, &m_offscreenAttachments.draws[i].vertexBuffer, m_offscreenAttachments.draws[i].pVertexOffset);

		vkCmdBindIndexBuffer(m_offscreenAttachments.cmdBuffers[i],
			m_offscreenAttachments.draws[i].indexBuffer,
			m_offscreenAttachments.draws[i].indexOffset,
			m_offscreenAttachments.draws[i].indexType);

		vkCmdDrawIndexed(m_offscreenAttachments.cmdBuffers[i],
			m_offscreenAttachments.draws[i].indexCount,
			m_offscreenAttachments.draws[i].indexCount,
			m_offscreenAttachments.draws[i].firstIndex,
			m_offscreenAttachments.draws[i].vertexOffset,
			m_offscreenAttachments.draws[i].firstInstance);


		vkCmdEndRenderPass(m_offscreenAttachments.cmdBuffers[i]);
		VK_CHECK_RESULT(vkEndCommandBuffer(m_offscreenAttachments.cmdBuffers[i]));

		

	}
}

void CRenderer::initRessources()
{
	//vkDebug::
	//m_dfb = new CFramebuffer();
	//m_offscreenTargets.push_back(new CFramebuffer());
	
	addOffscreen("font");

	std::vector<VkDescriptorPoolSize> poolSize;

	//Set layout bindings creation
	/*std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		//vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		//vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
		vkTools::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};


	std::vector<VkVertexInputBindingDescription> bindings = { vkTools::initializers::vertexInputBindingDescription(0, sizeof(VertexT), VK_VERTEX_INPUT_RATE_VERTEX) };

	std::vector<VkVertexInputAttributeDescription> attributes =	{ vkTools::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		vkTools::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float)) };
	
	std::string shaderName = "texture";

	addShader(gEnv->getAssetpath()+"shaders/texture.vert.spv", gEnv->getAssetpath() + "shaders/texture.frag.spv",
		&shaderName,setLayoutBindings, bindings, attributes);
	*/
	gEnv->pRessourcesManager->prepareShaders();
	//Descriptor Pool creation

	/*for (size_t i = 0; i < setLayoutBindings.size();i++) {
		requestDescriptorSet(setLayoutBindings[i].descriptorType, 1);
	}*/

	poolSize.resize(m_shaders.poolSize.size());

	for (size_t i = 0; i < poolSize.size();i++) {
		poolSize[i] = m_shaders.poolSize[i];
	}

	/*poolSize.resize(setLayoutBindings.size()+1+m_shaders.poolSize.size());
	for (size_t i = 0; i < setLayoutBindings.size();i++) {
		poolSize[i] = vkTools::initializers::descriptorPoolSize(setLayoutBindings[i].descriptorType, 1);
	}

/*	poolSize[setLayoutBindings.size()] = 
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);*/
	/*for (size_t i = setLayoutBindings.size()+1; i < poolSize.size();i++) {
		poolSize[i] = m_shaders.poolSize[i-setLayoutBindings.size()-1];
	}*/

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo =
		vkTools::initializers::descriptorPoolCreateInfo((uint32_t)poolSize.size(), poolSize.data(), static_cast<uint32_t>(poolSize.size()));
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_shaders.descriptorPool));
	//m_shaders.shaders.back()->load(m_device);
	
	getShader("texture")->load(m_device);
	printf("Shader color loaded\n");
	getShader("color")->load(m_device);
	getShader("tex")->load(m_device);
	//createDescriptorSet(getDescriptorPool(0), getShader("color")->getDescriptorSetLayoutPtr(), 1, 1);
	//std::vector<VkDescriptorSet>::iterator it = m_shaders.descriptorSets.begin();


	//m_shaders.shaders.back()->attachDescriptorSet(&(*it));
	
	//m_shaders.shaders.back()->attachDescriptorSet(&m_shaders.descriptorSets.back());
//	m_shaders.shaders.back()->attachDescriptorSet(m_shaders.descriptorSets.size()-1-m_shaders.poolSize.size());

	//addDescriptorSet(m_shaders.descriptorPool, m_shaders.shaders.back()->getDescriptorSetLayoutPtr(), 1); //#changed lately
	//m_shaders.shaders.back()->attachDescriptorSet(uint32_t(0));

	//printf("%i\n", &m_shaders.descriptorSets[0]);
	addRenderPass("main");
	addRenderPass("offscreen");
	//addRenderPass("gui");

	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("texture"),
	//	gEnv->pRenderer->getRenderPass("main"), "texture",
	//	0,
	//	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	//	VK_POLYGON_MODE_FILL,
	//	2);

	

	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("texture"),
		gEnv->pRenderer->getRenderPass("main"), "texture", false);

	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass("gui"), "gui");
	
	//gEnv->pRenderer->addRenderPass("gui");
	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass("gui"), "gui");

	gEnv->pMemoryManager->allocateMemory();

	m_offscreenTargets[0]->load(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), getRenderPass("offscreen"));
	
	//m_dfb->load(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(),getRenderPass("offscreen"));

}

void CRenderer::handleMessages(WPARAM wParam, LPARAM lParam)
{
	/*dev_data.rotationZ += 20.0f;
	dev_updateUniform_2();
	printf("here");*/

	/*switch (wParam) {
	case 0x50:
		//printf("here");
		dev_data.rotationZ += 1.0f;
		dev_updateUniform_2();
		break;
	case 0x42: //B
		printf("%s\n",buffersLayoutToString().c_str());
		break;
	}*/
}

void CRenderer::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = 
	{
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo((uint32_t)poolSizes.size(), poolSizes.data(), 1);

	//VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));
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
		renderPassBeginInfo.framebuffer = m_framebuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vkTools::initializers::viewport((float)gEnv->pSystem->getWidth(), (float)gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i],0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);

		//vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, dev_data.pipelineLayout, 0, 1, &dev_data.descriptorSet, 0, NULL);
		//vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, dev_data.shader->getPipelineLayout(), 0, 1, &dev_data.descriptorSet, 0, NULL);
		//vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, getShader("basic")->getPipelineLayout(), 0, 1, m_shaders.descriptorSets.back(), 0, NULL);
		vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, getShader("basic")->getPipelineLayout(), 0, 1, &m_shaders.descriptorSets[getShader("basic")->getDescriptorSetId()], 0, NULL);
		vkCmdBindPipeline(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, getPipeline("devp"));

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

void CRenderer::createBuffer(uint64_t * id, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size)
{
	m_buffers.push_back({});
	if (id != nullptr) {
		*id = static_cast<uint64_t>(m_buffers.size()-1);
	}
	m_buffers.back().device = m_device;
	m_buffers.back().usageFlags = usageFlags;
	m_buffers.back().memoryPropertyFlags = memoryFlags;
	m_buffers.back().size = size;

	createBuffer(m_buffers.back().usageFlags, m_buffers.back().memoryPropertyFlags, size, 0, &m_buffers.back().buffer, &m_buffers.back().memory);
}

void CRenderer::bufferSubData(uint64_t id, VkDeviceSize size, VkDeviceSize offset, void * data)
{
	writeInBuffer(&m_buffers[id].buffer, size, data, offset);
}

void CRenderer::createTexture(uint32_t * id, VkImageCreateInfo imageCreateInfo, void* datas, uint32_t width, uint32_t height)
{
	vkTools::VulkanTexture tex = {};
	tex.width = width;
	tex.height = height;
	//tex.imageLayout = 
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo allocInfo = vkTools::initializers::memoryAllocateInfo();

	if (m_vulkanDevice->enableDebugMarkers) {
		vkDebug::setObjectName(m_vulkanDevice->logicalDevice, (uint64_t)tex.image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "texture" + (m_textures.size()-1));
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
	*id = static_cast<uint32_t>(m_textures.size());
}

void CRenderer::loadTextureFromFile(uint32_t * dstTexId, std::string filepath, VkFormat format)
{
	gli::texture2D tex(gli::load(filepath));
	assert(!tex.empty());

 	VkImageCreateInfo imageCreateInfo = vkTools::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_BC2_UNORM_BLOCK;
	imageCreateInfo.mipLevels = tex.levels();
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	imageCreateInfo.extent = { (uint32_t)tex.dimensions().x, (uint32_t)tex.dimensions().y, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	//*uint8_t* data;
	//memcpy(data, tex.data(), tex.size());

	gEnv->pRenderer->createTexture(dstTexId, imageCreateInfo, (uint8_t*)tex.data(), (uint32_t)tex.dimensions().x, (uint32_t)tex.dimensions().y);
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

uint32_t CRenderer::getRenderAttachementFramebufferOffset(uint32_t id)
{
	if (id>=m_renderAttachments.framebufferOffsets.size()) {
		printf("\nISSUE\n");
		assert(0);
	}
	uint32_t off = 0;
	for (size_t i = 0; i < static_cast<size_t>(id);i++) {
		off += m_renderAttachments.framebufferOffsets[i];
	}
	return off;
}

void CRenderer::createRenderPass(VkRenderPass* renderPass,
	VkAttachmentLoadOp colorLoadOp,
	VkAttachmentLoadOp depthLoadOp,
	bool colorUndefined,
	bool depthUndefined)
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	// Don't clear the framebuffer (like the renderpass from the example does)
	//attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].loadOp = colorLoadOp;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	if (colorUndefined) {
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	if (!colorUndefined) {
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment
	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = depthLoadOp;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	if (depthUndefined) {
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	if (!depthUndefined) {
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Use subpass dependencies for image layout transitions
	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;

	/*subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;*/

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;
	
	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, renderPass));
}

void CRenderer::prepared()
{
	m_prepared = true;
}

VkFramebuffer CRenderer::dev_fb()
{
	//return m_dfb->getFramebuffer();
	return m_offscreenTargets[0]->getFramebuffer();
}

void CRenderer::dev_offscreenSemaphore()
{
	VkSemaphoreCreateInfo info = vkTools::initializers::semaphoreCreateInfo();
	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &info, nullptr, &m_offscreenSemaphore));
}

COffscreenTarget CRenderer::getOffscreenTarget(std::string name)
{
	
	for (size_t i = 0; i < m_offscreenInfos.names.size();i++) {
		if (m_offscreenInfos.names[i]==name) {
			return m_offscreenInfos.targets[i];
		}
	}

	return COffscreenTarget();
}

uint64_t CRenderer::getOffscreenTargetId(std::string name)
{
	for (size_t i = 0; i < m_offscreenInfos.names.size(); i++) {
		if (m_offscreenInfos.names[i] == name) {
			return i;
		}
	}

	return UINT64_MAX;
}

void CRenderer::dev_test(float x, float y, float w, float h, float depth)
{


	std::vector<Vertex> tmpV = {
	{ { x + w, y + h, depth },{ 1.0f, 1.0f, 0.0f} },
	{ { x, y + h, depth },{ 0.0f, 1.0f, 0.0f} },
	{ { x, y, depth },{ 0.0f, 1.0f, 1.0f} },
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

	/*dev_data.shader = new vkTools::CShader("./data/shaders/basic.vert.spv", "./data/shaders/basic.frag.spv", setLayoutBindings, dev_data.bindingDescriptions, dev_data.attributeDescriptions);
	dev_data.shader->load(m_device);*/
	std::string shaderName = "basic";
	addShader("./data/shaders/basic.vert.spv", "./data/shaders/basic.frag.spv", &shaderName, setLayoutBindings, dev_data.bindingDescriptions, dev_data.attributeDescriptions);
	m_shaders.shaders.back()->load(m_device);
	//dev_data.shader->load(m_device);
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

			//std::vector<VkPipelineShaderStageCreateInfo> shaderStages = dev_data.shader->getShaderStages();

			/*VkGraphicsPipelineCreateInfo pipelineCreateInfo =
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

			addGraphicsPipeline(pipelineCreateInfo, dev_data.shader->getInputState(), "devp");*/

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vkTools::initializers::pipelineCreateInfo(
			&m_pipelines.pipelinesState.back(),
			m_shaders.shaders.back()->getPipelineLayout(),
			m_renderPass,
			0,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			2,
			//shaderStages.data());
			m_shaders.shaders.back()->getShaderStagesPtr());

	//std::cout << &dev_data.shader->getShaderStages() << std::endl;

	addGraphicsPipeline(pipelineCreateInfo, m_shaders.shaders.back()->getInputState(), "devp");

	writeInBuffer(&m_smem.buf, sizeof(dev_data.uboVS), &dev_data.uboVS, vsize + isize);

	dev_updateUniform_2();


	dev_data.uniformDataVS.descriptor.offset = vsize + isize;
	dev_data.uniformDataVS.descriptor.range = sizeof(dev_data.uboVS);
	dev_data.uniformDataVS.descriptor.buffer = m_smem.buf;

	dev_setupDescriptorSet();

	//#future delete this block
	/*dev_data.v_offsets[0] = 0;
	dev_data.indexedDraw = {};
	dev_data.indexedDraw.bindDescriptorSets(getShader("basic")->getPipelineLayout(), 1, getDescriptorSet(getShaderId("basic")));
	dev_data.indexedDraw.bindPipeline(getPipeline("devp"));
	dev_data.indexedDraw.bindVertexBuffers(m_smem.buf, 1, dev_data.v_offsets);
	dev_data.indexedDraw.bindIndexBuffer(m_smem.buf, (uint32_t)dev_data.vertices.size() * sizeof(Vertex), VK_INDEX_TYPE_UINT32);
	dev_data.indexedDraw.drawIndexed((uint32_t)dev_data.indices.size(), 1, 0, 0, 0);

	addIndexedDraw(dev_data.indexedDraw);*/

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

	std::string sname = "basic";//Shader name
	uint32_t shaderId = 0;
	addShader("./data/shaders/basic.vert.spv", "./data/shaders/basic.frag.spv", &sname,setLayoutBindings,dev_data.bindingDescriptions, dev_data.attributeDescriptions);
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

//	addDescriptorSet(m_shaders.descriptorPool, dev_data.shader->getDescriptorSetLayoutPtr(), 1);

	addDescriptorSet( m_shaders.descriptorPool, m_shaders.shaders.back()->getDescriptorSetLayoutPtr(), 1);
	m_shaders.shaders.back()->attachDescriptorSet(static_cast<uint32_t>(m_shaders.shaders.size()-1));
	//printf("%i\tad\n", &m_shaders.descriptorSets[0]);
/*	VkDescriptorSetAllocateInfo allocInfo =
		vkTools::initializers::descriptorSetAllocateInfo(m_descriptorPool, dev_data.shader->getDescriptorSetLayoutPtr(), 1);

	VkResult vkRes = vkAllocateDescriptorSets(m_device, &allocInfo, &dev_data.descriptorSet);
	assert(!vkRes);*/

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		//Binding 0 for Vertex Shader Uniform Buffer
		vkTools::initializers::writeDescriptorSet(
			//dev_data.descriptorSet,
			m_shaders.descriptorSets.back(),
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&dev_data.uniformDataVS.descriptor),
	};

	addWriteDescriptorSet(writeDescriptorSets);
	
	updateDescriptorSets();
	
	//vkUpdateDescriptorSets(m_device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
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
	if (m_smem.buf != nullptr) {
		vkDestroyBuffer(m_device, m_smem.buf, nullptr);

		vkFreeMemory(m_device, m_smem.mem, nullptr);
	
	vkDestroyPipelineLayout(m_device, dev_data.pipelineLayout, nullptr);

	}
}

VkResult CRenderer::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = gEnv->pSystem->getAppName().c_str();
	appInfo.pEngineName = gEnv->pSystem->getAppName().c_str();
	//appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 30);
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
