/*
* Assorted commonly used Vulkan helper functions
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#pragma warning (disable : 4996)
#include "vulkan/vulkan.h"

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#if defined(_WIN32)
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#elif defined(__ANDROID__)
#include "vulkanandroid.h"
#include <android/asset_manager.h>
#endif

#define VK_FLAGS_NONE 0

#define DEFAULT_FENCE_TIMEOUT 100000000000

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << vkTools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}	

namespace vkTools {
	class CShader {
	public:
		CShader(std::string vertexpath, std::string fragmentpath, std::vector<VkDescriptorSetLayoutBinding> descriptorLayout, std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription);
		//CShader();
		~CShader();
		void load(VkDevice device);
		void clear(VkDevice device);
		std::vector<VkShaderModule> getShaderModules() const;
		VkPipelineLayout getPipelineLayout() const; //@?Future ptr
		VkPipelineVertexInputStateCreateInfo getInputState() const; //@?Future ptr
		std::vector<VkPipelineShaderStageCreateInfo> getShaderStages();
		VkPipelineShaderStageCreateInfo* getShaderStagesPtr();
		VkDescriptorSetLayout* getDescriptorSetLayoutPtr();
		VkDescriptorSet* getDescriptorSetPtr();
		uint32_t getDescriptorSetId() const;
		void attachDescriptorSet(VkDescriptorSet* descriptorSet);
		void attachDescriptorSet(uint32_t id);

	private:
		
		std::vector<VkShaderModule> m_shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

		std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
		std::vector<VkVertexInputBindingDescription> m_bindingDescription;
		std::vector<VkVertexInputAttributeDescription> m_attributeDescription;
	
		std::vector <std::string> m_paths;
		std::vector <VkShaderStageFlagBits> m_flags;

		VkDescriptorSetLayoutCreateInfo m_descriptorLayoutCreateInfo;
		VkPipelineVertexInputStateCreateInfo m_inputState;
		VkPipelineLayoutCreateInfo m_pipelineLayoutCreateInfo;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;

		VkDescriptorSet* m_descriptorSet=nullptr;//This is the attached descriptorSet of the shader
		uint32_t m_descriptorSetId = UINT32_MAX;

		void addShader(std::string sPath, VkShaderStageFlagBits flag);

		void setInputState(uint32_t bindingCount, VkVertexInputBindingDescription *bindingDescriptions, uint32_t attributeCount, VkVertexInputAttributeDescription *attributeDescriptions);
		


	};

	struct VkPipelineState {
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		VkPipelineColorBlendAttachmentState blendAttachmentState;
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineMultisampleStateCreateInfo multisampleState;

		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicState;

	};

}


namespace vkTools
{

	VkBool32 checkDeviceExtensionPresent(VkPhysicalDevice physicalDevice, const char* extensionName);

	//Return string representation of a vulakn error string
	std::string errorString(VkResult errorCode);

	VkResult checkResult(VkResult result);

	VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);

	//Display error message and exit on fatal error
	void exitFatal(std::string message, std::string caption);

	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout);

	char *readBinaryFile(const char *filename, size_t *psize);

	VkShaderModule loadShader(const char *filename, VkDevice device, VkShaderStageFlagBits stage);

	struct UniformData
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
		uint32_t allocSize;
		void* mapped = nullptr;
	};


	void destroyUniformData(VkDevice device, vkTools::UniformData *uniformData);

	namespace equal {
		bool vertexAttributeInputDescriptor(const VkVertexInputAttributeDescription& viad1, const VkVertexInputAttributeDescription& viad2);
	}
	namespace initializers {

		VkAttachmentDescription attachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
			VkAttachmentLoadOp loadOp,
			VkAttachmentStoreOp storeOp,
			VkAttachmentLoadOp stencilLoadOp,
			VkAttachmentStoreOp stencilStoreOp,
			VkImageLayout initialLayout,
			VkImageLayout finalLayout);

		VkMemoryAllocateInfo memoryAllocateInfo();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t bufferCount);

		VkCommandBufferBeginInfo commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo();

		VkSemaphoreCreateInfo semaphoreCreateInfo();
		VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags);
		VkSubmitInfo submitInfo();

		VkImageMemoryBarrier imageMemoryBarrier();

		VkImageCreateInfo imageCreateInfo();
		VkSamplerCreateInfo samplerCreateInfo();
		VkImageViewCreateInfo imageViewCreateInfo();

		VkFramebufferCreateInfo framebufferCreateInfo();

		VkViewport viewport(float width , float height, float minDepth, float maxDepth);

		VkRect2D rect(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY);

		VkBufferCreateInfo bufferCreateInfo();
		VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags usage, VkDeviceSize size);

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
				uint32_t poolSizeCount,
				VkDescriptorPoolSize* pPoolSizes,
				uint32_t maxSets);

		VkDescriptorPoolSize descriptorPoolSize(
				VkDescriptorType type,
				uint32_t descriptorCount);

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
				VkDescriptorType type,
				VkShaderStageFlags stageFlags,
				uint32_t binding);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
				const VkDescriptorSetLayoutBinding* pBindings,
				uint32_t bindingCount);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t setLayoutCount);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
				VkDescriptorPool descriptorPool,
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t descriptorSetCount);

		VkDescriptorImageInfo descriptorImageInfo(
				VkSampler sampler,
				VkImageView imageView,
				VkImageLayout imageLayout);

		VkWriteDescriptorSet writeDescriptorSet(
				VkDescriptorSet dstSet,
				VkDescriptorType type,
				uint32_t binding,
				VkDescriptorBufferInfo* bufferInfo
		);

		VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo* imageInfo
		);

		VkCopyDescriptorSet copyDescriptorSet(VkDescriptorSet src, uint32_t srcBinding, VkDescriptorSet dst, uint32_t dstBinding, uint32_t descriptorCount);

		VkVertexInputBindingDescription vertexInputBindingDescription(
				uint32_t binding,
				uint32_t stride,
				VkVertexInputRate inputRate);

		VkVertexInputAttributeDescription vertexInputAttributeDescription(
				uint32_t binding,
				uint32_t location,
				VkFormat format,
				uint32_t offset);


		VkGraphicsPipelineCreateInfo pipelineCreateInfo(
			VkPipelineState* dst,
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkPipelineCreateFlags flags,
			VkPrimitiveTopology topology,
			VkPolygonMode polyMode,
			uint32_t shaderStagesCount,
			VkPipelineShaderStageCreateInfo* shaderStages);

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology,
			VkPipelineInputAssemblyStateCreateFlags flags,
			VkBool32 primitiveRestartEnable);

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
				VkPolygonMode polygonMode,
				VkCullModeFlags cullMode,
				VkFrontFace frontFace,
				VkPipelineRasterizationStateCreateFlags flags);

		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWritemask,
			VkBool32 blendEnable);

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
				uint32_t attachmentCount,
				const VkPipelineColorBlendAttachmentState * pAttachments);

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
				VkBool32 depthTestEnable,
				VkBool32 depthWriteEnable,
				VkCompareOp depthCompareOp);

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
				uint32_t viewportCount,
				uint32_t scissorCount,
				VkPipelineViewportStateCreateFlags flags);

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
				VkSampleCountFlagBits rasterizationSamples, 
				VkPipelineMultisampleStateCreateFlags flags);

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
				const VkDynamicState * pDynamicStates,
				uint32_t dynamicStateCount,
				VkPipelineDynamicStateCreateFlags flags);

		VkPipelineTessellationStateCreateInfo pipelineTesselationStateCreateInfo(uint32_t patchControlPoints);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo(
				VkPipelineLayout layout,
				VkRenderPass renderPass,
				VkPipelineCreateFlags flags);

	}

}