#include "OffscreenTarget.h"



COffscreenTarget::COffscreenTarget()
{
}


COffscreenTarget::~COffscreenTarget()
{
	vkDestroyImageView(gEnv->getDevice(), m_color.view, nullptr);
	vkDestroyImage(gEnv->getDevice(), m_color.img, nullptr);
	vkFreeMemory(gEnv->getDevice(), m_color.mem, nullptr);

	vkDestroyImageView(gEnv->getDevice(), m_depth.view, nullptr);
	vkDestroyImage(gEnv->getDevice(), m_depth.img, nullptr);
	vkFreeMemory(gEnv->getDevice(), m_depth.mem, nullptr);
}

void COffscreenTarget::load(uint32_t width, uint32_t height, VkRenderPass renderPass, VkFormat colorFormat)
{
	m_width = width;
	m_height = height;
	m_renderPass = renderPass;
	m_colorFormat = colorFormat;
	prepareOffscreen();
}

VkRenderPass COffscreenTarget::getRenderPass()
{
	return m_renderPass;
}

VkFramebuffer COffscreenTarget::getFrameBuffer()
{
	return m_framebuffer;
}

uint32_t COffscreenTarget::getWidth()
{
	return m_width;
}

uint32_t COffscreenTarget::getHeight()
{
	return m_height;
}

void COffscreenTarget::prepareOffscreen()
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

	VK_CHECK_RESULT(vkCreateImage(gEnv->getDevice(), &image, nullptr, &m_color.img));
	vkGetImageMemoryRequirements(gEnv->getDevice(), m_color.img, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = gEnv->pRenderer->getVulkanDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(gEnv->getDevice(), &memAlloc, nullptr, &m_color.mem));
	VK_CHECK_RESULT(vkBindImageMemory(gEnv->getDevice(), m_color.img, m_color.mem, 0));

	VkImageViewCreateInfo colorImageView = vkTools::initializers::imageViewCreateInfo();
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = m_colorFormat;
	colorImageView.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 1;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = m_color.img;
	VK_CHECK_RESULT(vkCreateImageView(gEnv->getDevice(), &colorImageView, nullptr, &m_color.view));

	VkSamplerCreateInfo samplerInfo = vkTools::initializers::samplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR; //#warning may use nearest for precise operation
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
	VK_CHECK_RESULT(vkCreateSampler(gEnv->getDevice(), &samplerInfo, nullptr, &m_sampler));

	image.format = fbDepthFormat;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VK_CHECK_RESULT(vkCreateImage(gEnv->getDevice(), &image, nullptr, &m_depth.img));
	vkGetImageMemoryRequirements(gEnv->getDevice(), m_depth.img, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = gEnv->pRenderer->getVulkanDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(gEnv->getDevice(), &memAlloc, nullptr, &m_depth.mem));
	VK_CHECK_RESULT(vkBindImageMemory(gEnv->getDevice(), m_depth.img, m_depth.mem, 0));

	VkImageViewCreateInfo depthStencilView = vkTools::initializers::imageViewCreateInfo();
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = fbDepthFormat;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = m_depth.img;
	VK_CHECK_RESULT(vkCreateImageView(gEnv->getDevice(), &depthStencilView, nullptr, &m_depth.view));

	VkImageView attachments[2];
	attachments[0] = m_color.view;
	attachments[1] = m_depth.view;

	m_framebuffer = gEnv->pRenderer->addFramebuffer(m_width, m_height, m_renderPass, 2, attachments);

}
