#include "Framebuffer.h"



CFramebuffer::CFramebuffer()
{
	draw_data.bufferId = gEnv->pMemoryManager->requestMemory(4 * sizeof(VertexT) + (6 * sizeof(uint32_t)),
		"Framebuffer (4*Vertex + 6*uint32_t)");
	//draw_data.bufferOffset = gEnv->pMemoryManager->requestMemory(4 * sizeof(VertexT) + (6 * sizeof(uint32_t)),
	//	"Framebuffer (4*Vertex + 6*uint32_t)");
		//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	draw_data.descriptorSetId = gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, "texture");
}


CFramebuffer::~CFramebuffer()
{
	vkDestroyImageView(gEnv->getDevice(), m_color.view, nullptr);
	vkDestroyImage(gEnv->getDevice(), m_color.image, nullptr);
	vkFreeMemory(gEnv->getDevice(), m_color.mem, nullptr);

	vkDestroyImageView(gEnv->getDevice(), m_depth.view, nullptr);
	vkDestroyImage(gEnv->getDevice(), m_depth.image, nullptr);
	vkFreeMemory(gEnv->getDevice(), m_depth.mem, nullptr);

	//vkDestroyFramebuffer(gEnv->getDevice(), m_framebuffer, nullptr);

	//vkDestroySemaphore(gEnv->getDevice(), m_semaphore, nullptr);
	//vkFreeCommandBuffers(gEnv->getDevice(), gEnv->pRenderer->getCommandPool(), 1, &m_cmdBufferOffscreen);

}

void CFramebuffer::load(uint32_t width, uint32_t height, VkRenderPass renderPass, VkFormat colorFormat)
{
	m_width = width;
	m_height = height;
	m_renderPass = renderPass;
	m_colorFormat = colorFormat;
	prepareOffscreen();
	prepareDescriptorSet();
	gEnv->pRenderer->dev_offscreenSemaphore();
	//createOffscreenSemaphore();
	//createOffscreenCommandBuffer();
}

VkFramebuffer CFramebuffer::getFramebuffer() const
{
	return m_framebuffer;
}

VkSemaphore CFramebuffer::getSemaphore() const
{
	return m_semaphore;
}

VkSemaphore* CFramebuffer::getSemaphorePtr()
{
	return &m_semaphore;
}

VkCommandBuffer CFramebuffer::getCmdBuffer() const
{
	return m_cmdBufferOffscreen;
}

VkCommandBuffer* CFramebuffer::getCmdBufferPtr()
{
	return &m_cmdBufferOffscreen;
}

void CFramebuffer::prepareOffscreen()
{
	VkFormat fbDepthFormat;
	VkBool32 validDepthFormat = vkTools::getSupportedDepthFormat(gEnv->pRenderer->getVulkanDevice()->physicalDevice, &fbDepthFormat);
	assert(validDepthFormat);

	VkImageCreateInfo image = vkTools::initializers::imageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = m_colorFormat;
	image.extent.width = m_width;
	image.extent.height = m_height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = vkTools::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &image, nullptr, &m_color.image));
	vkGetImageMemoryRequirements(gEnv->pRenderer->getVulkanDevice()->logicalDevice, m_color.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = gEnv->pRenderer->getVulkanDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &memAlloc, nullptr, &m_color.mem));
	VK_CHECK_RESULT(vkBindImageMemory(gEnv->pRenderer->getVulkanDevice()->logicalDevice, m_color.image, m_color.mem, 0));

	//COLOR ATTACHEMNT

	VkImageViewCreateInfo colorImageView = vkTools::initializers::imageViewCreateInfo();
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = m_colorFormat;
	colorImageView.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = m_color.image;
	VK_CHECK_RESULT(vkCreateImageView(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &colorImageView, nullptr, &m_color.view));

	VkSamplerCreateInfo samplerInfo = vkTools::initializers::samplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = samplerInfo.addressModeU;
	samplerInfo.addressModeW = samplerInfo.addressModeU;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 0;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &samplerInfo, nullptr, &m_sampler));

	//DEPTH ATTACHEMNT

	image.format = fbDepthFormat;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VK_CHECK_RESULT(vkCreateImage(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &image, nullptr, &m_depth.image));
	vkGetImageMemoryRequirements(gEnv->pRenderer->getVulkanDevice()->logicalDevice, m_depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = gEnv->pRenderer->getVulkanDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &memAlloc, nullptr, &m_depth.mem));
	VK_CHECK_RESULT(vkBindImageMemory(gEnv->pRenderer->getVulkanDevice()->logicalDevice, m_depth.image, m_depth.mem, 0));

	VkImageViewCreateInfo depthStencilView = vkTools::initializers::imageViewCreateInfo();
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = fbDepthFormat;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = m_depth.image;
	VK_CHECK_RESULT(vkCreateImageView(gEnv->pRenderer->getVulkanDevice()->logicalDevice, &depthStencilView, nullptr, &m_depth.view));

	//Framebuffer Description

	VkImageView attachments[2];
	attachments[0] = m_color.view;
	attachments[1] = m_depth.view;

	m_framebuffer = gEnv->pRenderer->addFramebuffer(m_width, m_height, m_renderPass, 2, attachments);
}

void CFramebuffer::prepareDescriptorSet()
{

	//std::array<VertexT,4> vertData = { VertexT(-0.5f, -0.5f, 0.1f, 0.0f,0.0f), VertexT(-0.5f, 0.5f, 0.1f, 0.0f,1.0f), VertexT(0.5f, 0.5f, 0.1f, 1.0f,1.0f), VertexT(0.5f, -0.5f,0.1f, 1.0f,0.0f) };

	std::array<VertexT, 4> vertData = { VertexT(-1.0f, -1.0f, 0.1f, 0.0f,0.0f), VertexT(-1.0f, 1.0f, 0.1f, 0.0f,1.0f), VertexT(1.0f, 1.0f, 0.1f, 1.0f,1.0f), VertexT(1.0f, -1.0f,0.1f, 1.0f,0.0f) };
	uint32_t indices[6] = {0,1,2,0,2,3};
	size_t tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(draw_data.bufferId).bufferInfo.offset;
	gEnv->pRenderer->bufferSubData(gEnv->bbid, 4*sizeof(VertexT),tmpOffset, vertData.data() );
	gEnv->pRenderer->bufferSubData(gEnv->bbid, 6 * sizeof(uint32_t), tmpOffset + (4*sizeof(VertexT)), indices);

	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0),gEnv->pRenderer->getShader("texture")->getDescriptorSetLayoutPtr(), 1, draw_data.descriptorSetId);

	draw_data.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	draw_data.imageInfo.imageView = m_color.view;
	draw_data.imageInfo.sampler = m_sampler;

	std::vector<VkWriteDescriptorSet> writeDescriptor = {
		vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(draw_data.descriptorSetId), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &draw_data.imageInfo)
	};
	gEnv->pRenderer->addWriteDescriptorSet(writeDescriptor);
	gEnv->pRenderer->updateDescriptorSets();

	draw_data.quad = {};
	draw_data.quad.bindDescriptorSets(gEnv->pRenderer->getShader("texture")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(draw_data.descriptorSetId));
	draw_data.quad.bindPipeline(gEnv->pRenderer->getPipeline("texture"));
	//draw_data.gOffset[0] = draw_data.bufferOffset;
	draw_data.gOffset[0] = tmpOffset;
	draw_data.quad.bindVertexBuffers(gEnv->pRenderer->getBuffer(gEnv->bbid),1,draw_data.gOffset);
	draw_data.quad.bindIndexBuffer(gEnv->pRenderer->getBuffer(gEnv->bbid), sizeof(VertexT)*4+tmpOffset, VK_INDEX_TYPE_UINT32);
	draw_data.quad.drawIndexed(6, 1, 0, 0, 0);

	//gEnv->pRenderer->addIndexedDraw(draw_data.quad, gEnv->pRenderer->getRenderPass("main"));

	//gEnv->pRenderer->addDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("texture")->getDescriptorSetLayoutPtr(), 1);

}

void CFramebuffer::createOffscreenSemaphore()
{
	VkSemaphoreCreateInfo info = vkTools::initializers::semaphoreCreateInfo();

	VK_CHECK_RESULT(vkCreateSemaphore(gEnv->getDevice(),&info, nullptr, &m_semaphore));

}

void CFramebuffer::createOffscreenCommandBuffer()
{
	//VkCommandBufferAllocateInfo allocInfo = vkTools::initializers::commandBufferAllocateInfo(gEnv->pRenderer->getCommandPool);
	m_cmdBufferOffscreen = gEnv->pRenderer->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
}
