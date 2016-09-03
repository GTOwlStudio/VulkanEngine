#pragma once

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;

#include <Windows.h>
#include "vulkandevice.h"

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

struct SIndexedDrawInfo
{
	VkDescriptorSet* descriptorSets; //explicit
	uint32_t descriptorCount;
	VkPipelineLayout* pipelineLayout; //A pointer to a pipeline layout
	VkPipeline* pipeline; //A pointer to a pipeline
	VkBuffer* vertexBuffer; //The buffer where the data are located
	VkDeviceSize* pVertexOffset; //The offset inside the buffer where the data are located

	VkBuffer* indexBuffer; //The buffer where the indices are located (it CAN be the same as the vertexBuffer)
	VkDeviceSize indexOffset; //The offset inside the buffer where the indices are located
	VkIndexType indexType; //The type used for the indices, uint32_t etc
	uint32_t indexCount; //is the number of vertices to draw.
	uint32_t instanceCount; //is the number of instances to draw.
	uint32_t firstIndex; //is the base index within the index buffer.
	uint32_t vertexOffset; //is the value added to the vertex index before indexing into the vertex buffer.
	uint32_t firstInstance; //It's the instance ID of the first instance to draw.

	void bindDescriptorSets(VkPipelineLayout *paramPipelineLayout, uint32_t paramDescriptorCount,VkDescriptorSet* pDescriptorSets) {
		pipelineLayout = paramPipelineLayout;
		descriptorCount = paramDescriptorCount;
		descriptorSets = pDescriptorSets;
	}

	void bindPipeline(VkPipeline *p_pipeline) {
		pipeline = p_pipeline;
	}

	void bindVertexBuffers() 
	{

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
		VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo const& inpuState, std::string name) = 0;

	virtual void addShader(std::string vsPath, std::string fsPath, std::string *shaderName, std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings, std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription)=0;
	
	virtual void addWriteDescriptorSet(std::vector<VkWriteDescriptorSet> writeDescriptorSets) = 0;

	virtual void updateDescriptorSets() = 0;

	virtual void addIndexedDraw(SIndexedDrawInfo drawInfo) = 0;
	virtual void buildDrawCommands() = 0;

	virtual void createTexture(uint32_t* id, VkImageCreateInfo imageCreateInfo, uint8_t* datas, uint32_t width, uint32_t height) = 0;
	
	virtual void createBuffer(uint32_t* id, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size) = 0;

	virtual void bufferSubData(uint32_t bufferID, VkDeviceSize size, VkDeviceSize offset, void*  data) = 0;

	//Maybe Not final function
	virtual void handleMessages(WPARAM wParam, LPARAM lParam) = 0;

	virtual vk::VulkanDevice* getVulkanDevice() = 0;
	virtual vkTools::VulkanTextureLoader* getTextureLoader() = 0;
	virtual vkTools::CShader* getShader(std::string shaderName) = 0;
	virtual VkBuffer getBuffer(uint32_t id)= 0;
	
};