#pragma once

#include "System.h"
#include <vulkan\vulkan.h>
#include "vulkanTools\vulkantools.h"
#include "vulkanTools\vulkanTextureLoader.hpp"



class CFramebuffer
{
public:
	CFramebuffer();
	~CFramebuffer();
	void load(uint32_t width, uint32_t height, VkRenderPass renderPass, VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM);
	VkFramebuffer getFramebuffer() const;
	VkSemaphore getSemaphore() const;
	VkSemaphore* getSemaphorePtr();
	VkCommandBuffer getCmdBuffer() const;
	VkCommandBuffer* getCmdBufferPtr();
	VkImage getColorImageAttachmentImage();
	VkImage getDepthImageAttachmentImage();
	
protected:

	struct FrameBufferAttachment{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};

	int32_t m_width, m_height;
	VkFramebuffer m_framebuffer;
	FrameBufferAttachment m_color, m_depth;
	VkSampler m_sampler;
	VkFormat m_colorFormat;

	uint32_t m_texId;

	VkRenderPass m_renderPass;

	struct {
		//VkDeviceSize bufferOffset; //Offset of the fisrt data of the object in the buffer
		size_t bufferId;
		SIndexedDrawInfo quad;
		VkDeviceSize gOffset[1];
		//VertexT m_quadcoord[6];
		uint32_t descriptorSetId;
		VkDescriptorImageInfo imageInfo;
	} draw_data;
	//Semaphore used to synchronise between the offscreen and final scene render pass
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
	VkCommandBuffer m_cmdBufferOffscreen;

	void prepareOffscreen();
	void prepareDescriptorSet();
	void createOffscreenSemaphore();
	void createOffscreenCommandBuffer();
		
};

