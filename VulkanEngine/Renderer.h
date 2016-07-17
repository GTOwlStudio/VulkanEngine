#pragma once

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif
#include <array>

#include <vulkan\vulkan.h>
#include "vulkanTools\vulkanswapchain.hpp"
#include "vulkanTools\vulkandebug.h"
#include "IRenderer.h"

#include "System.h"

class CSystem;

class CRenderer : public IRenderer
{
public:
	CRenderer();
	~CRenderer();
	virtual void InitVulkan();
	virtual void Init();

	virtual void render();

	VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex);
	uint32_t getMemoryType(uint32_t typeBits, VkFlags properties);

protected:
	
	
	void createCommandPool();

	void createCommandBuffers();

	void setupDepthStencil();
	void setupRenderPass();

	void createPipelineCache();

	void setupFrameBuffer();

	void createSetupCommandBuffer();
	void flushSetupCommandBuffer();

	void buildPresentCommandBuffers();

	void initSwapChain();
	void setupSwapChain();

	void destroyCommandBuffer();

	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);
	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);

protected:
	
	void draw();
	void prepareFrame();
	void submitFrame();

	virtual void addGraphicPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo inputState, std::string name);
	

	void buildCommandBuffer();

	void createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);

	void createSBuffer(VkDeviceSize size, void* data);


protected:
	VkInstance m_instance;
	
	struct {
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	} m_physicalDevice;
	VkDevice m_device;
	VkQueue m_queue;

	VkRenderPass m_renderPass;

	VkCommandPool m_cmdPool;

	std::vector<VkCommandBuffer> m_drawCmdBuffers;
	std::vector<VkFramebuffer> m_frameBuffers;

	uint32_t m_currentBuffer = 0;

	std::vector<VkCommandBuffer> m_postPresentCmdBuffers = {VK_NULL_HANDLE};
	std::vector<VkCommandBuffer> m_prePresentCmdBuffers = {VK_NULL_HANDLE};

	VulkanSwapChain m_swapChain;



	struct {
		VkSemaphore presentComplete;
		VkSemaphore renderComplete;
	} m_semaphores;

	struct {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} m_depthStencil;


	VkSubmitInfo m_submitInfo;
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkFormat m_depthFormat;
	VkFormat m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

	VkCommandBuffer m_setupCmdBuffer = VK_NULL_HANDLE;

	VkPipelineCache m_pipelineCache;
	std::vector<VkPipeline> m_pipelines;
	std::vector<std::string> m_pipelineName;

	struct{
		VkBuffer buf;
		VkDeviceMemory mem;
	} m_smem;

private:
	VkResult createInstance();
	VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation);

	


};

