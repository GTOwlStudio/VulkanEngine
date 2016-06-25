#pragma once
#include <stdio.h>
#include <assert.h>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_TO_ZERO

#include <glm\glm.hpp>
#include <gli\gli.hpp>

#include <vulkan\vulkan.h>
#include "TEngineBase.h"


struct Vertex {
	float pos[3];
	float uv[2];
};


struct Texture {
	VkSampler sampler;
	VkImage image;
	VkImageLayout imageLayout;
	VkDeviceMemory deviceMemory;
	VkImageView view;
	uint32_t width, height;
	uint32_t mipLevels;
};

#define VERTEX_BUFFER_BIND_ID 0

class TEngine : public TEngineBase 
{
public:
	TEngine(bool enableValidation);
	~TEngine();

	void draw();

	virtual void prepare();
	virtual void render();
protected:

	Texture texture;

	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		uint32_t count;
	} indices;

	vkTools::UniformData uniformDataVS;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
		float lodBias = 0.0f;
	} uboVS;

	struct {
		/*VkPipeline dtext; //Pipeline use to render text that is update frequentely
		VkPipeline faces; //Pipeline use to display faces		 (triangle)
		VkPipeline edges; //Pipeline use to display edges		 (lines)
		VkPipeline vertices; //Pipelines use to display vertices (points)*/
		VkPipeline solid;
	} pipelines;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;


	void buildCommandBuffers();
	void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags ascpectMask,
		VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	void generateCoord(float x, float y, float w, float h, float depth);
	void setupVertexDescription();
	void prepareUniformBuffers();
	void updateUniformBuffers();

	void setupDescriptorSetLayout();
	void setupDescriptorPool();
	void setupDescriptorSet();

	void preparePipelines();

	void loadTexture(std::string filename, VkFormat format, bool forceLinearTiling);

	void destroyTextureImage(Texture texture);

	virtual void viewChanged();
};