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

#include "vulkandevice.hpp"
#include "System.h"

typedef VkPhysicalDeviceFeatures (*PFN_GetEnabledFeatures)();

#define VERTEX_BUFFER_BIND_ID 0

class CSystem;
class CShader;
class CRenderer;

struct Vertex {
	float pos[3];
	float col[3];
};

class CRenderer : public IRenderer
{
public:
	CRenderer(PFN_GetEnabledFeatures enabledFeaturesFn = nullptr);
	//CRenderer();
	~CRenderer();
	virtual void InitVulkan();
	virtual void Init();

	virtual void render();

	VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex);
	uint32_t getMemoryType(uint32_t typeBits, VkFlags properties);
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);

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

	virtual void addGraphicPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo const& inputState, std::string name);
	virtual void handleMessages(WPARAM wParam, LPARAM lParam);

	void setupDescriptorPool();
	void buildCommandBuffer();

	VkBool32 createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);
	VkBool32 createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);
	VkBool32 createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory, VkDescriptorBufferInfo * descriptor);
	void createSBuffer(VkDeviceSize size, void* data);
	void writeInBuffer(VkBuffer*buffer, VkDeviceSize size, void* data, VkDeviceSize dstOffset);


protected:

	bool m_enableDebugMarkers = false;
	bool m_enableVSync = false;

	VkPhysicalDeviceFeatures m_enabledFeatures = {};

	VkInstance m_instance;
	
	vk::VulkanDevice *m_vulkanDevice;

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

	VkDescriptorPool m_descriptorPool;

	VkPipelineCache m_pipelineCache;
	std::vector<VkPipeline> m_pipelines; 
	std::vector<vkTools::VkPipelineState> m_pipelinesState;
	VkPipeline dev_pipeline;
	std::vector<std::string> m_pipelineName;

	struct{
		VkBuffer buf;
		VkDeviceMemory mem;
	} m_smem;

	
	void dev_test(float x, float y, float w, float h, float depth);
	void clean_dev();

	void dev_setupDescriptorSet();
	void dev_prepareUBO();
	void dev_updateUniform();
	void dev_updateUniform_2();

	struct {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		VkPipelineLayout pipelineLayout;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo inputState;
		VkDescriptorSet descriptorSet;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkShaderModule> shaderModules;

		vkTools::CShader *shader;

		struct {
			glm::mat4 projection;
			glm::mat4 view;
		} uboVS;

		vkTools::UniformData uniformDataVS;

		float rotationSpeed = 1.0f;
		float rotationZ = 0.0f;

	} dev_data;

	bool m_prepared = false;

private:
	VkResult createInstance();
	VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation);

	


};

