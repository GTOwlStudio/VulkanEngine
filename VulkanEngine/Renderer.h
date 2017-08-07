#pragma once

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif
#include <array>
#include <inttypes.h>

#include <vulkan\vulkan.h>
#include "vulkanTools\vulkanswapchain.hpp"
#include "vulkanTools\vulkandebug.h"
#include "vulkanTools\vulkanTextureLoader.hpp"
#include "vulkanTools\Helper.h"
#include "IRenderer.h"


#include "vulkandevice.h"
#include "System.h"
#include "Framebuffer.h"
#include "OffscreenTarget.h"

typedef VkPhysicalDeviceFeatures (*PFN_GetEnabledFeatures)();

#define VERTEX_BUFFER_BIND_ID 0


class CSystem;
class vkTools::CShader;
class CRenderer;
class CFramebuffer;
class COffscreenTarget;

struct Vertex {
	float pos[3];
	float col[3];
};


struct VertexT {
	
	glm::vec3 pos;
	glm::vec2 tc;
	VertexT(float x, float y, float z, float tx, float ty) {
		pos = glm::vec3(x, y, z);
		tc = glm::vec2(tx, ty);
		/*pos[0] = x;
		pos[1] = y;
		pos[2] = z;
		tc[0] = tx;
		tc[1] = ty;*/
	}
	VertexT():VertexT(0.0f,0.0f,0.0f,0.0f,0.0f){}
};


class CRenderer : public IRenderer
{
public:
	CRenderer(PFN_GetEnabledFeatures enabledFeaturesFn = nullptr);
	//CRenderer();
	~CRenderer();
	void clearRessources();

	virtual void InitVulkan();
	virtual void Init();

	virtual void render();

	virtual vk::VulkanDevice* getVulkanDevice();
	virtual vkTools::VulkanTextureLoader* getTextureLoader();
	virtual VkRenderPass getRenderPass(std::string renderPassName);
	virtual vkTools::CShader* getShader(std::string shaderName);
	virtual vkTools::CShader* getShader(uint32_t id);
	virtual uint32_t getShaderId(std::string shaderName);
	virtual size_t getShaderLastBinding();

	virtual uint64_t getBufferAvaibleId();

	virtual VkBuffer getBuffer(uint64_t id);
	virtual vk::Buffer* getBufferStruct(uint32_t id);
	virtual vkTools::VulkanTexture* getTexture(uint32_t texId);
	virtual VkDescriptorSet* getDescriptorSet(uint32_t id);
	virtual VkPipeline getPipeline(std::string pipelineName);
	virtual VkDescriptorPool getDescriptorPool(uint32_t id);
	virtual VkCommandPool getCommandPool();
	virtual CFramebuffer* getOffscreen(std::string name);

	virtual uint32_t requestDescriptorSet(VkDescriptorType type, uint32_t descriptorCount, std::string descriptorLayoutName);
	virtual uint32_t requestDescriptorSet(std::vector<VkDescriptorType> types, uint32_t descriptorCount, std::string descriptorLayoutName);

	virtual void getInfo();
	virtual void getBufferInfo();

	virtual void bcb();
	virtual void graphicsInit(); //Initializeand prepare graphics object for the drawing, such as clearing view image, etc
	virtual void prepared(); //Set m_prepared to true

	virtual VkFramebuffer dev_fb();
	virtual void dev_offscreenSemaphore();

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

	//void buildPresentCommandBuffers();

	void initSwapChain();
	void setupSwapChain();

	void destroyCommandBuffer();

	bool checkCommandBuffers();

	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);
	void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);

protected:

	void draw();
	void prepareFrame();
	void submitFrame();

	virtual void addGraphicsPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo const& inputState, std::string name);
	virtual void addGraphicsPipeline(
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		VkPipelineCreateFlags flags,
		VkPrimitiveTopology topology,
		VkPolygonMode polyMode,
		uint32_t shaderStagesCount,
		VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo const& inpuState, std::string name);

	virtual void addGraphicsPipeline(vkTools::CShader* shader, VkRenderPass renderPass,std::string name,
		bool blend,
		VkPipelineCreateFlags flags,
		VkPrimitiveTopology topology,
		VkPolygonMode polyMode,
		uint32_t shaderStagesCount);

	virtual void addRenderPass(std::string renderPassName, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
	virtual void addRenderPass(std::string renderPassName, VkAttachmentDescription colorAttachmentDescription);
	virtual void addShader(std::string vsPath, std::string fsPath, std::string *shaderName,
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings,
		std::vector<VkVertexInputBindingDescription> bindingDescription,
		std::vector<VkVertexInputAttributeDescription> attributeDescription);
	virtual VkDescriptorSet addDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount);
	virtual void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount, VkDescriptorSet* dstDescriptor);
	virtual void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount, uint32_t descriptorId); //descriptorId is the id of the descriptor in the m_shaders.descriptorSet[id]
	virtual void addWriteDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets);
	virtual void addCopyDescriptorSet(std::vector<VkCopyDescriptorSet> copyDescriptorSets);
	virtual VkFramebuffer addFramebuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, uint32_t attachmentCount, VkImageView *pAttachments);
	virtual CFramebuffer* addOffscreen(std::string name);

	virtual void updateDescriptorSets();

	void loadShaders();

	virtual void addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::string tag="none");
	virtual void addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::vector<VkFramebuffer> framebuffers);
	virtual void addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, VkFramebuffer framebuffer);
	virtual void addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass, std::string targetName);
	virtual void addDrawToDrawList(uint64_t drawId, std::string tag);
	virtual void swap(uint64_t ida, uint64_t idb);
	virtual void buildDrawCommands(VkRenderPass renderPass);
	virtual void buildDrawCommands(); 
	virtual void buildDrawCommands_old();
	virtual void buildOffscreenDrawCommands();
	virtual void buildTargetedDrawCommands();

	virtual void initRessources();

	virtual void handleMessages(WPARAM wParam, LPARAM lParam);

	virtual void createBuffer(uint64_t *id, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size);

	virtual void bufferSubData(uint64_t id, VkDeviceSize size, VkDeviceSize offset, void* data);

	void setupDescriptorPool();
	void buildCommandBuffer();

	VkBool32 createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);
	VkBool32 createBuffer(VkBufferUsageFlags usage, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory);
	VkBool32 createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory, VkDescriptorBufferInfo * descriptor);

	virtual void createTexture(uint32_t* id, VkImageCreateInfo imageCreateInfo, void* datas, uint32_t width, uint32_t height);
	virtual void loadTextureFromFile(uint32_t* dstTexId, std::string filepath, VkFormat format);

	void createSBuffer(VkDeviceSize size, void* data);
	void writeInBuffer(VkBuffer*buffer, VkDeviceSize size, void* data, VkDeviceSize dstOffset);

	uint32_t getRenderAttachementFramebufferOffset(uint32_t id);

	void createRenderPass(VkRenderPass* renderPass, 
		VkAttachmentLoadOp colorLoadOp, 
		VkAttachmentLoadOp depthLoadOp, 
		bool colorUndefined = false, 
		bool depthUndefined = false); //DepthToo set depth to loadOp value

protected:

	bool m_enableDebugMarkers = true;
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
	std::vector<VkFramebuffer> m_framebuffers;
//	std::vector<VkBuffer> m_buffer;
		
	uint32_t m_currentBuffer = 0;

/*	std::vector<VkCommandBuffer> m_postPresentCmdBuffers = {VK_NULL_HANDLE};
	std::vector<VkCommandBuffer> m_prePresentCmdBuffers = {VK_NULL_HANDLE};*/

	VulkanSwapChain m_swapChain;



	struct {
		VkSemaphore presentComplete; //Swap chain image presentation
		VkSemaphore renderComplete; //Command buffer submision and execution
	} m_semaphores;

	//std::vector<VkFence> m_waitFences;

	struct {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} m_depthStencil;


	VkSubmitInfo m_submitInfo;
	//VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkFormat m_depthFormat;
	VkFormat m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

	VkCommandBuffer m_setupCmdBuffer = VK_NULL_HANDLE;

	
//	VkDescriptorPool m_descriptorPool;
	std::vector<VkWriteDescriptorSet> m_writeDescriptorSets; //writeDescriptorSets to write, via the method updateDescriptorSets
	std::vector<VkCopyDescriptorSet> m_copyDescriptorSets;


	struct {
		std::vector<VkRenderPass> renderPasses;
		std::vector<std::string> names;
		std::array<VkAttachmentDescription, 2> attachementDescriptions;
		VkAttachmentReference colorReference;
		VkAttachmentReference depthReference;
		VkSubpassDescription subpassDescription;
		std::array<VkSubpassDependency, 2> subpassDependencies;
	} m_renderPasses;

	struct {
		VkPipelineCache pipelineCache;
		std::vector<VkPipeline> pipelines;
		std::vector<vkTools::VkPipelineState> pipelinesState;
		VkPipeline dev_pipeline;
		std::vector<std::string> pipelineNames;
	} m_pipelines;

	
	struct {
		std::vector<vkTools::CShader*> shaders;
		std::vector<std::string> names;
		std::vector<VkDescriptorSet> descriptorSets;
		//std::vector<VkDescriptorType> descriptorTypes;
		std::vector<std::string> descriptorLayoutNames;
		//std::vector<VkDescriptorSet>
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorPoolSize> poolSize;
	} m_shaders;


	std::vector<vk::Buffer> m_buffers;
	std::vector<vkTools::VulkanTexture> m_textures;

	std::vector<SIndexedDrawInfo> m_indexedDraws;

	std::vector<uint64_t> m_drawsList;
	std::vector<std::string> m_drawsListTags;
	
	struct {
		std::vector<VkRenderPass> renderPasses; //Render pass used for rendering
		std::vector<VkFramebuffer> framebuffers; //Framebuffer Target, the render will go in this, you might have, multi
		std::vector<uint32_t> framebufferOffsets; /*It's if you use multiple buffering (such as we do for rendering swaping buffer etc), If you wan't to use double buffering, you need to have 2 framebuffer as target,
		you add your 2 framebuffers to renderTarget.framebuffers vector (A VkFramebuffer object is in fact a pointer so you can do like fbA = fbB) and you add the value 2 to framebufferOffsets*/
		std::vector<bool> isOffscreen;//Offscreen is rendered separately so you have to say if this an offscreen or not
	} m_renderAttachments;

	struct {
		std::vector<COffscreenTarget> targets;
		std::vector<std::string> names;
	} m_offscreenInfos;

	COffscreenTarget getOffscreenTarget(std::string name);
	uint64_t getOffscreenTargetId(std::string name);

	struct {
		std::vector<SIndexedDrawInfo> draws;
		std::vector<std::string> targetNames;
		std::vector<VkCommandBuffer> cmdBuffers;
	} m_offscreenAttachments;
	
	
	vkTools::VulkanTextureLoader* m_textureLoader = nullptr;


	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
	} m_smem;

	void dev_test(float x, float y, float w, float h, float depth);
	void dev_test2(float x, float y, float w, float h, float depth);
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

		VkDeviceSize v_offsets[1];
		SIndexedDrawInfo indexedDraw;

	} dev_data;

	std::vector<CFramebuffer*> m_offscreenTargets;
	std::vector<std::string> m_offscreenNames;
	VkCommandBuffer m_offscreenCmdBuffer = VK_NULL_HANDLE;
	VkSemaphore m_offscreenSemaphore = VK_NULL_HANDLE;
	bool m_offscreen = false;

	bool m_prepared = false;

	VkRenderPass m_initRenderPass[2];

private:
	VkResult createInstance();

	std::string buffersLayoutToString();

	//VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool enableValidation);

	


};

