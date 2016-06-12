#pragma once

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <chrono>
#include <vector>
#include <array>


#include <vulkan\vulkan.h>
#include "vulkantools.h"
#include "vulkandebug.h"

#include "vulkanMeshLoader.hpp"
#include "vulkanswapchain.hpp"

class TEngineBase
{
public:
	TEngineBase(bool enableValidation);
	~TEngineBase();

	bool prepared = false;
	uint32_t width = 1280;
	uint32_t height = 720;

	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	float zoom = 0;

	float timer = 0.0f;

	float timerSpeed = 0.25f;

	bool paused = false;

	float rotationSpeed = 1.0f;
	float zoomSpeed = 1.0f;

	glm::vec3 rotation = glm::vec3();
	glm::vec3 cameraPos = glm::vec3();
	glm::vec2 mousePos;


	std::string title = "TEngine by Tema (based on Vulkan API)";
	std::string name = "tEngine";

	struct
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} depthStencil;

#if defined(_WIN32)
	HWND window;
	HINSTANCE windowInstance;
#endif

	void initVulkan(bool enableValidation);

#if defined(_WIN32)
	void setupConsole(std::string title);
	HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
	void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	virtual void render() = 0;

	virtual void viewChanged();
	virtual void keyPressed(uint32_t keyCode);
	virtual void windowResized();

	std::string getWindowTitle();

	VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex);
	uint32_t getMemoryType(uint32_t typeBits, VkFlags properties);

	//Create a new command pool object storing command buffers
	void createCommandPool();

	void setupDepthStencil();
	virtual void setupFrameBuffer();

	virtual void setupRenderPass();


	void initSwapchain();
	void setupSwapchain();


	void createCommandBuffers();
	// Destroy all command buffers and set their handles to VK_NULL_HANDLE
	// May be necessary during runtime if options are toggled 
	void destroyCommandBuffers();

	//Create command for setup command
	void createSetupCommandBuffer();

	void createPipelineCache();

	void flushSetupCommandBuffer();

	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);

	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);

	virtual void prepare();

	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);

	VkBool32 createBuffer(VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize size,
		void *data,
		VkBuffer *buffer,
		VkDeviceMemory *memory);

	VkBool32 createBuffer(VkBufferUsageFlags usage,
		VkDeviceSize size,
		void *data,
		VkBuffer *buffer,
		VkDeviceMemory *memory);

	VkBool32 createBuffer(VkBufferUsageFlags usage,
		VkDeviceSize size,
		void *data,
		VkBuffer *buffer,
		VkDeviceMemory *memory,
		VkDescriptorBufferInfo *descriptor);

	void loadMesh(
		std::string filename,
		vkMeshLoader::MeshBuffer *meshBuffer,
		std::vector<vkMeshLoader::VertexLayout> vertexLayout,
		float scale);

	void renderLoop();

	void submitPrePresentBarrier(VkImage image);

	void submitPostPresentBarrier(VkImage image);

	VkDevice getDevice() const;
	const std::string getAssetPath();
	const VkRenderPass getRenderPass();
	const VkPipelineCache getPipelineCache();
protected:
	float frameTimer = 1.0f;

	uint32_t frameCounter = 0;

	//Vulkan Instance, stores all per-application states
	VkInstance instance;
	//Physical device(GPU) that Vulkan will use
	VkPhysicalDevice physicalDevice;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

	VkDevice device;

	VkQueue queue;

	VkFormat colorformat = VK_FORMAT_B8G8R8A8_UNORM;

	VkFormat depthFormat;
	//Command buffer pool
	VkCommandPool cmdPool;
	//Command buffer used for setup
	VkCommandBuffer setupCmdBuffer = VK_NULL_HANDLE;
	VkCommandBuffer postPresentCmdBuffer = VK_NULL_HANDLE;
	VkCommandBuffer prePresentCmdBuffer = VK_NULL_HANDLE;
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submitInfo;

	std::vector<VkCommandBuffer> drawCmdBuffers;

	VkRenderPass renderPass;

	std::vector<VkFramebuffer> frameBuffers;

	uint32_t currentBuffer = 0;

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	std::vector<VkShaderModule> shaderModules;


	VkPipelineCache pipelineCache;

	VulkanSwapChain swapChain;

	struct {
		VkSemaphore presentComplete;
		VkSemaphore renderComplete;
	} semaphores;

	

private:

	bool enableValidation = false;

	float fpsTimer = 0.0f;

	VkResult createInstance(bool enableValidation);
	VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation);



	uint32_t destWidth;
	uint32_t destHeight;

	void windowResize();

};

