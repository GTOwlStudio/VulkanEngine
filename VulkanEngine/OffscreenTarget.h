#pragma once

#include "System.h"
#include "vulkan_header.h"

class COffscreenTarget
{
public:
	COffscreenTarget();
	~COffscreenTarget();
	void load(uint32_t width, uint32_t height, VkRenderPass renderPass, VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM);
	VkRenderPass getRenderPass();
	VkFramebuffer getFrameBuffer();
	uint32_t getWidth();
	uint32_t getHeight();

protected:
	void prepareOffscreen();
	struct FramebufferAttachment {
		VkImage img;
		VkDeviceMemory mem;
		VkImageView view;
	};

	uint32_t m_width, m_height;
	VkFramebuffer m_framebuffer;
	FramebufferAttachment m_color, m_depth;//#enhancement flexibility
	VkSampler m_sampler;
	VkFormat m_colorFormat;

	uint32_t m_texId;
	VkRenderPass m_renderPass;

};

