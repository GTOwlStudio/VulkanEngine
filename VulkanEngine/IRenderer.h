#pragma once

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;

#include <Windows.h>
#include "vulkandevice.h"
//#include "System.h"

///**/
//
//struct SWindGrid 
//{
//	int m_nWidth;
//	int m_nHeight;
//	float m_fCellSize;
//	
//	vec3 m_vCentr;
//
//	SWindGrid() {};
//};
//
//class CRenderCamera
//{
//public:
//	CRenderCamera();
//	CRenderCamera(const CRenderCamera& Cam);
//	void Copy(const CRenderCamera& Cam);
//
//	void LookAt(const vec3& Eye, const vec3& ViewRefpt, const vec3& ViewUp);
//	void Perspective(float YFov, float Aspect, float NDist, float FDist);
//	vec3 ViewDir() const;
//	vec3 ViewDirOffAxis() const;
//
//	void Translate(const vec3& trans);
//	void Rotate(const mat3);
//
//	vec3 vX, vY, vZ;
//	vec3 vOrigin;
//	float fWL, fWR, fWB, fWT; //W for viewport and L,R,B,T for left, roght, bottom, top
//	float fNear, fFar;
//
//};
//
//CRenderCamera::CRenderCamera()
//{
//	vX = vec3(1, 0, 0);
//	vY = vec3(0, 1, 0);
//	vZ = vec3(0, 0, 1);
//}
//
//CRenderCamera::CRenderCamera(const CRenderCamera& Cam)
//{
//	Copy(Cam);
//}
//
//void CRenderCamera::Copy(const CRenderCamera& Cam) {
//	vX = Cam.vX;
//	vY = Cam.vY;
//	vZ = Cam.vZ;
//	vOrigin = Cam.vOrigin;
//	fNear = Cam.fNear;
//	fFar = Cam.fFar;
//	fWL = Cam.fWL;
//	fWR = Cam.fWR;
//	fWB = Cam.fWB;
//	fWT = Cam.fWT;
//}
//
//void CRenderCamera::LookAt(const vec3& Eye, const vec3& ViewRefpt, const vec3& ViewUp) {
//	
//}

class CSystem;
class CFramebuffer;

struct SIndexedDrawInfo
{
	VkDescriptorSet* descriptorSets; //explicit
	uint32_t descriptorCount;
	VkPipelineLayout pipelineLayout; //A pointer to a pipeline layout
	VkPipeline pipeline; //pipeline
	VkBuffer vertexBuffer; //The buffer where the data are located
	uint32_t offsetCount;
	VkDeviceSize* pVertexOffset; //The offset inside the buffer where the data are located

	VkBuffer indexBuffer; //The buffer where the indices are located (it CAN be the same as the vertexBuffer)
	VkDeviceSize indexOffset; //The offset inside the buffer where the indices are located
	VkIndexType indexType; //The type used for the indices, uint32_t etc
	uint32_t indexCount; //is the number of vertices to draw.
	uint32_t instanceCount; //is the number of instances to draw.
	uint32_t firstIndex; //is the base index within the index buffer.
	uint32_t vertexOffset; //is the value added to the vertex index before indexing into the vertex buffer.
	uint32_t firstInstance; //It's the instance ID of the first instance to draw.

	void bindDescriptorSets(VkPipelineLayout paramPipelineLayout, uint32_t paramDescriptorCount,VkDescriptorSet* pDescriptorSets) {
		pipelineLayout = paramPipelineLayout;
		descriptorCount = paramDescriptorCount;
		descriptorSets = pDescriptorSets;
	}

	void bindPipeline(VkPipeline p_pipeline) {
		pipeline = p_pipeline;
	}

	void bindVertexBuffers(VkBuffer p_pBuffers, uint32_t p_offsetCount, VkDeviceSize* p_pOffset) {
		vertexBuffer = p_pBuffers;
		offsetCount = p_offsetCount;
		pVertexOffset = p_pOffset;
	}

	void bindIndexBuffer(VkBuffer p_pBuffer, VkDeviceSize p_offset, VkIndexType p_type) {
		indexBuffer = p_pBuffer;
		indexOffset = p_offset;
		indexType = p_type;
	}

	void drawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset, uint32_t p_firstInstance)
	{
		indexCount = p_indexCount;
		instanceCount = p_instanceCount;
		firstIndex = p_firstIndex;
		vertexOffset = p_vertexOffset;
		firstInstance = p_firstInstance;
	}

};

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void Init() = 0;
	virtual void InitVulkan() = 0;
	virtual void render() = 0;
	virtual void addGraphicsPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo const& inputState, std::string name) = 0;
	virtual void addGraphicsPipeline(
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		VkPipelineCreateFlags flags,
		VkPrimitiveTopology topology,
		VkPolygonMode polyMode,
		uint32_t shaderStagesCount,
		VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo const& inputState, std::string name) = 0;
	virtual void addGraphicsPipeline(vkTools::CShader* shader, VkRenderPass renderPass, std::string name,
		bool blend = false,
		VkPipelineCreateFlags flags = 0,
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VkPolygonMode polyMode = VK_POLYGON_MODE_FILL,
		uint32_t shaderStagesCount = 2) = 0;

	virtual void addRenderPass(std::string renderPassName, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR) = 0;
	virtual void addRenderPass(std::string renderPassName, VkAttachmentDescription colorAttachmentDescription) = 0;
	virtual void addShader(std::string vsPath, std::string fsPath, std::string *shaderName, std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings, std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription)=0;
	virtual VkDescriptorSet addDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount) = 0;
	virtual void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount, VkDescriptorSet* dstDescriptor) = 0;
	virtual void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* pDescriptorLayout, uint32_t descriptorLayoutCount, uint32_t id) = 0;
	virtual void addWriteDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets) = 0;
	virtual VkFramebuffer addFramebuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, uint32_t attachmentCount, VkImageView *pAttachments) = 0;
	virtual CFramebuffer* addOffscreen(std::string name) = 0;

	virtual VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin) = 0;

	virtual void updateDescriptorSets() = 0;

	virtual void addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass) = 0;
	virtual void addIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::vector<VkFramebuffer> framebuffers) = 0;
	virtual void addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, VkFramebuffer framebuffer = VK_NULL_HANDLE) = 0;
	virtual void addOffscreenIndexedDraw(SIndexedDrawInfo drawInfo, VkRenderPass renderPass, std::string targetName) = 0;
	virtual void buildDrawCommands(VkRenderPass renderPass) = 0;
	virtual void buildDrawCommands() = 0;

	virtual void createTexture(uint32_t* id, VkImageCreateInfo imageCreateInfo, uint8_t* datas, uint32_t width, uint32_t height) = 0;
	
	virtual void createBuffer(uint64_t* id, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size) = 0;

	virtual void bufferSubData(uint64_t bufferID, VkDeviceSize size, VkDeviceSize offset, void*  data) = 0;

	virtual void handleMessages(WPARAM wParam, LPARAM lParam) = 0; //#supposition maybe not a final function

	virtual vk::VulkanDevice* getVulkanDevice() = 0;
	virtual vkTools::VulkanTextureLoader* getTextureLoader() = 0;
	virtual VkRenderPass getRenderPass(std::string renderPassName) = 0;
	virtual vkTools::CShader* getShader(std::string shaderName) = 0; //Return an adress to the shader named 'shaderName'
	virtual vkTools::CShader* getShader(uint32_t id) = 0;
	virtual uint32_t getShaderId(std::string shaderName) = 0; //Return the id of the shader named 'shaderName'
															  //return UINT32_MAX if there is no shader named 'shader_name'
	virtual size_t getShaderLastBinding() = 0;

	virtual uint64_t getBufferAvaibleId() = 0;// Return an avaible Id

	virtual VkBuffer getBuffer(uint64_t id)= 0;
	virtual vk::Buffer* getBufferStruct(uint32_t id) = 0;
	virtual vkTools::VulkanTexture* getTexture(uint32_t texId) = 0;
	virtual VkDescriptorSet* getDescriptorSet(uint32_t) = 0;
	virtual VkPipeline getPipeline(std::string pipelineName) = 0;
	virtual VkDescriptorPool getDescriptorPool(uint32_t id) = 0;
	virtual VkCommandPool getCommandPool() = 0;
	//virtual CFramebuffer* getOffscreen(uint32_t id) = 0;
	virtual CFramebuffer* getOffscreen(std::string name) = 0;

	virtual uint32_t requestDescriptorSet(VkDescriptorType type, uint32_t descriptorCount, std::string descriptorLayoutName = "null") = 0;

	virtual void getInfo() = 0;
	virtual void getBufferInfo() = 0;
	virtual void bcb()=0; //Build Command Buffer #dev too heavy
	virtual void prepared() = 0; //Set m_prepared = true

	virtual VkFramebuffer dev_fb() = 0;
	virtual void dev_offscreenSemaphore() = 0;

};