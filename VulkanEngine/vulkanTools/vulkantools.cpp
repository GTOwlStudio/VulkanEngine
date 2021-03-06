/*
* Assorted commonly used Vulkan helper functions
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "vulkantools.h"

namespace vkTools {
	VkBool32 checkDeviceExtensionPresent(VkPhysicalDevice physicalDevice, const char * extensionName)
	{
		uint32_t extensionCount = 0;
		std::vector<VkExtensionProperties> extensions;

		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		extensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
		/*vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
		extensions.resize(extensionCount);
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions.data());*/
		for (auto& ext : extensions)
		{
			if (!strcmp(extensionName, ext.extensionName))
			{
				return true;
			}
		}
		return false;
	}
	std::string errorString(VkResult errorCode)
	{
		switch (errorCode)
		{
#define STR(r) case VK_ ##r: return #r
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			STR(ERROR_VALIDATION_FAILED_EXT);
			//STR(ERROR_INVALID_SHADER_NV);
#undef STR
		default:
			return "UNKNOWN_ERROR";
		}
	}

	VkResult checkResult(VkResult result) {
		if(result != VK_SUCCESS) {
			std::string errorMsg = "Fatal : VkResult returned " + errorString(result) + "!";
			std::cout << errorMsg << std::endl;
			assert(result==VK_SUCCESS);
		}
		return result;
	}

	void exitFatal(std::string message, std::string caption)
	{
#ifdef _WIN32
		MessageBox(NULL, message.c_str(), caption.c_str(), MB_OK | MB_ICONERROR);
#endif
		std::cerr << message << "\n";
		exit(1);
	}

	VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				*depthFormat = format;
				return true;
			}
		}

		return false;
	}

	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageFlags,
		VkPipelineStageFlags destStageFlags)
	{
		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier = vkTools::initializers::imageMemoryBarrier();
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)

		// Undefined layout
		// Only allowed as initial layout!
		// Make sure any writes to the image have been finished

		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source 
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}

		switch (newImageLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		}



		/*if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		// Old layout is color attachment
		// Make sure any writes to the color buffer have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		// Old layout is depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		// Old layout is transfer source
		// Make sure any reads from the image have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// Old layout is shader read (sampler, input attachment)
		// Make sure any shader reads from the image have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

		// Target layouts (new)

		// New layout is transfer destination (copy, blit)
		// Make sure any copyies to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		// New layout is transfer source (copy, blit)
		// Make sure any reads from and writes to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// New layout is color attachment
		// Make sure any writes to the color buffer hav been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// New layout is depth attachment
		// Make sure any writes to depth/stencil buffer have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		// New layout is shader read (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		*/
		// Put barrier on top

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageFlags,
			destStageFlags,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	char *readBinaryFile(const char *filename, size_t *psize)
	{
		long int size;
		size_t retval;
		void *shader_code;

		FILE *fp = fopen(filename, "rb");
		if (!fp) return NULL;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		shader_code = malloc(size);
		retval = fread(shader_code, size, 1, fp);
		assert(retval == 1);

		*psize = size;

		return (char*)shader_code;
	}

	VkShaderModule loadShader(const char *fileName, VkDevice device, VkShaderStageFlagBits stage) {
		size_t size = 0;
		const char *shaderCode = readBinaryFile(fileName, &size);
		assert(size>0);

		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo moduleCreateInfo;
		VkResult err;

		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;

		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;
		moduleCreateInfo.flags = 0;
		err = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);
		assert(!err);

		return shaderModule;

	}

	void destroyUniformData(VkDevice device, vkTools::UniformData * uniformData)
	{
		if (uniformData->mapped != nullptr) {
			vkUnmapMemory(device, uniformData->memory);
		}
		vkDestroyBuffer(device, uniformData->buffer, nullptr);
		vkFreeMemory(device, uniformData->memory, nullptr);
	}

	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		setImageLayout(cmdbuffer, image, aspectMask, oldImageLayout, newImageLayout, subresourceRange);
	}

}

VkAttachmentDescription vkTools::initializers::attachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	return VkAttachmentDescription();
}

VkMemoryAllocateInfo vkTools::initializers::memoryAllocateInfo()
{
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = 0;
	memAllocInfo.memoryTypeIndex = 0;
	return memAllocInfo;
}

VkCommandBufferAllocateInfo vkTools::initializers::commandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t bufferCount)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = level;
	commandBufferAllocateInfo.commandBufferCount = bufferCount;
	return commandBufferAllocateInfo;
}

VkCommandBufferBeginInfo vkTools::initializers::commandBufferBeginInfo()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	return cmdBufferBeginInfo;
}

VkRenderPassBeginInfo vkTools::initializers::renderPassBeginInfo()
{
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	return renderPassBeginInfo;
}

VkSemaphoreCreateInfo vkTools::initializers::semaphoreCreateInfo()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;
	return semaphoreCreateInfo;
}

VkFenceCreateInfo vkTools::initializers::fenceCreateInfo(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = flags;
	return fenceCreateInfo;
}

VkSubmitInfo vkTools::initializers::submitInfo()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	return submitInfo;
}

VkImageMemoryBarrier vkTools::initializers::imageMemoryBarrier()
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return imageMemoryBarrier;
}

VkImageCreateInfo vkTools::initializers::imageCreateInfo()
{
	VkImageCreateInfo imgCreateInfo = {};
	imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgCreateInfo.pNext = NULL;
	return imgCreateInfo;
}

VkSamplerCreateInfo vkTools::initializers::samplerCreateInfo()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = NULL;
	return samplerCreateInfo;
}

VkImageViewCreateInfo vkTools::initializers::imageViewCreateInfo()
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.pNext = NULL;
	return imgViewCreateInfo;
}

VkFramebufferCreateInfo vkTools::initializers::framebufferCreateInfo()
{
	VkFramebufferCreateInfo fbCreateInfo = {};
	fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCreateInfo.pNext = NULL;
	return fbCreateInfo;
}

VkViewport vkTools::initializers::viewport(float width, float height, float minDepth, float maxDepth)
{
	VkViewport viewport = {};
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	return viewport;
}

VkRect2D vkTools::initializers::rect(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY)
{
	VkRect2D rect2D = {};
	rect2D.extent.width = width;
	rect2D.extent.height = height;
	rect2D.offset.x = offsetX;
	rect2D.offset.y = offsetY;
	return rect2D;
}

VkBufferCreateInfo vkTools::initializers::bufferCreateInfo()
{
	VkBufferCreateInfo bufCreateInfo = {};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	return bufCreateInfo;
}

VkBufferCreateInfo vkTools::initializers::bufferCreateInfo(VkBufferUsageFlags usage, VkDeviceSize size)
{
	VkBufferCreateInfo bufCreateInfo = {};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufCreateInfo.pNext = NULL;
	bufCreateInfo.usage = usage;
	bufCreateInfo.size = size;
	bufCreateInfo.flags = 0;

	return bufCreateInfo;
}

VkDescriptorPoolCreateInfo vkTools::initializers::descriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize * pPoolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = NULL;
	descriptorPoolInfo.poolSizeCount = poolSizeCount;
	descriptorPoolInfo.pPoolSizes = pPoolSizes;
	descriptorPoolInfo.maxSets = maxSets;
	return descriptorPoolInfo;
}

VkDescriptorPoolSize vkTools::initializers::descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = type;
	descriptorPoolSize.descriptorCount = descriptorCount;
	return descriptorPoolSize;
}

VkDescriptorSetLayoutBinding vkTools::initializers::descriptorSetLayoutBinding(
	VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
	VkDescriptorSetLayoutBinding setLayoutBinding = {};
	setLayoutBinding.descriptorType = type;
	setLayoutBinding.stageFlags = stageFlags;
	setLayoutBinding.binding = binding;

	setLayoutBinding.descriptorCount = 1;
	return setLayoutBinding;
}

VkDescriptorSetLayoutCreateInfo vkTools::initializers::descriptorSetLayoutCreateInfo(
	const VkDescriptorSetLayoutBinding * pBindings, uint32_t bindingCount)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutBinding = {};
	descriptorSetLayoutBinding.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutBinding.pNext = NULL;
	descriptorSetLayoutBinding.pBindings = pBindings;
	descriptorSetLayoutBinding.bindingCount = bindingCount;
	return descriptorSetLayoutBinding;
}

VkPipelineLayoutCreateInfo vkTools::initializers::pipelineLayoutCreateInfo(const VkDescriptorSetLayout * pSetLayouts, uint32_t setLayoutCount)
{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
	pPipelineLayoutCreateInfo.pSetLayouts  = pSetLayouts;
	return pPipelineLayoutCreateInfo;
}

VkDescriptorSetAllocateInfo vkTools::initializers::descriptorSetAllocateInfo(
	VkDescriptorPool descriptorPool, const VkDescriptorSetLayout * pSetLayouts, uint32_t descriptorSetCount)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
	descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
	return descriptorSetAllocateInfo;
}

VkDescriptorImageInfo vkTools::initializers::descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo descriptorImageinfo = {};
	descriptorImageinfo.sampler = sampler;
	descriptorImageinfo.imageView = imageView;
	descriptorImageinfo.imageLayout = imageLayout;
	return descriptorImageinfo;
}

VkWriteDescriptorSet vkTools::initializers::writeDescriptorSet(
	VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo * bufferInfo)
{
	VkWriteDescriptorSet writeDescriptor = {};
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.pNext = NULL;
	writeDescriptor.dstSet = dstSet;
	writeDescriptor.descriptorType = type;
	writeDescriptor.dstBinding = binding;
	writeDescriptor.pBufferInfo = bufferInfo;
	writeDescriptor.descriptorCount = 1;
	return writeDescriptor;
}

VkWriteDescriptorSet vkTools::initializers::writeDescriptorSet(
	VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo * imageInfo)
{
	VkWriteDescriptorSet writeDescriptor = {};
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.pNext = NULL;
	writeDescriptor.dstSet = dstSet;
	writeDescriptor.descriptorType = type;
	writeDescriptor.dstBinding = binding;
	writeDescriptor.pImageInfo = imageInfo;
	writeDescriptor.descriptorCount = 1;
	return writeDescriptor;
}

VkCopyDescriptorSet vkTools::initializers::copyDescriptorSet(VkDescriptorSet src, uint32_t srcBinding, VkDescriptorSet dst, uint32_t dstBinding, uint32_t descriptorCount)
{
	VkCopyDescriptorSet copyDescriptorSet;
	copyDescriptorSet.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
	copyDescriptorSet.pNext = NULL;
	copyDescriptorSet.srcSet = src;
	copyDescriptorSet.srcBinding = srcBinding;
	copyDescriptorSet.srcArrayElement = 0;
	copyDescriptorSet.dstSet = dst;
	copyDescriptorSet.dstBinding = dstBinding;
	copyDescriptorSet.dstArrayElement = 0;
	copyDescriptorSet.descriptorCount = descriptorCount;
	return copyDescriptorSet;
}



VkVertexInputBindingDescription vkTools::initializers::vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	VkVertexInputBindingDescription vInputBindDescription = {};
	vInputBindDescription.binding = binding;
	vInputBindDescription.stride = stride;
	vInputBindDescription.inputRate = inputRate;
	return vInputBindDescription;
}

VkVertexInputAttributeDescription vkTools::initializers::vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription vInputAttributeDescription = {};
	vInputAttributeDescription.location = location;
	vInputAttributeDescription.binding = binding;
	vInputAttributeDescription.format = format;
	vInputAttributeDescription.offset = offset;
	return vInputAttributeDescription;
}

VkGraphicsPipelineCreateInfo vkTools::initializers::pipelineCreateInfo(
	VkPipelineState* dst,
	VkPipelineLayout layout,
	VkRenderPass renderPass,
	VkPipelineCreateFlags flags,
	VkPrimitiveTopology topology, 
	VkPolygonMode polyMode,
	uint32_t shaderStagesCount,
	VkPipelineShaderStageCreateInfo * shaderStages)
{
	/*VkGraphicsPipelineCreateInfo pipelineCreateInfo = vkTools::initializers::pipelineCreateInfo(layout, renderPass, flags);

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(topology, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vkTools::initializers::pipelineRasterizationStateCreateInfo(polyMode,VK_CULL_MODE_NONE,VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
	VkPipelineColorBlendAttachmentState blendAttahcmentState =
		vkTools::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = 
		vkTools::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttahcmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState =
		vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), dynamicStateEnables.size(), 0);*/

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = vkTools::initializers::pipelineCreateInfo(layout, renderPass, flags);

	dst->inputAssemblyState = 
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(topology, 0, VK_FALSE);
	dst->rasterizationState = 
		vkTools::initializers::pipelineRasterizationStateCreateInfo(polyMode, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	dst->blendAttachmentState =
		vkTools::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	dst->colorBlendState =
		vkTools::initializers::pipelineColorBlendStateCreateInfo(1, &dst->blendAttachmentState);
	dst->depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	/*dst->depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);*/
	/*dst->depthStencilState.minDepthBounds = 1.0f;
	dst->depthStencilState.maxDepthBounds = 1.0f;*/
	dst->viewportState =
		vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	dst->multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	dst->dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	dst->dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(dst->dynamicStateEnables.data(), static_cast<uint32_t>(dst->dynamicStateEnables.size()), 0);


	pipelineCreateInfo.pInputAssemblyState = &dst->inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &dst->rasterizationState;
	pipelineCreateInfo.pColorBlendState = &dst->colorBlendState;
	pipelineCreateInfo.pMultisampleState = &dst->multisampleState;
	pipelineCreateInfo.pViewportState = &dst->viewportState;
	pipelineCreateInfo.pDepthStencilState = &dst->depthStencilState;
	pipelineCreateInfo.pDynamicState = &dst->dynamicState;
	pipelineCreateInfo.stageCount = shaderStagesCount;
	pipelineCreateInfo.pStages = shaderStages;


	return pipelineCreateInfo;
}

VkPipelineVertexInputStateCreateInfo vkTools::initializers::pipelineVertexInputStateCreateInfo()
{
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateinfo = {};
	pipelineVertexInputStateCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateinfo.pNext = NULL;
	return pipelineVertexInputStateCreateinfo;
}

VkPipelineInputAssemblyStateCreateInfo vkTools::initializers::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = topology;
	inputAssemblyState.flags = flags;
	inputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;
	return inputAssemblyState;
}

VkPipelineRasterizationStateCreateInfo vkTools::initializers::pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags)
{
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = polygonMode;
	rasterizationState.cullMode = cullMode;
	rasterizationState.frontFace = frontFace;
	rasterizationState.flags = flags;
	rasterizationState.depthClampEnable = VK_TRUE;
	rasterizationState.lineWidth = 1.0f;
	return rasterizationState;
}

VkPipelineColorBlendAttachmentState vkTools::initializers::pipelineColorBlendAttachmentState(VkColorComponentFlags colorWritemask, VkBool32 blendEnable)
{
	VkPipelineColorBlendAttachmentState blendAttachmentState = {};
	blendAttachmentState.colorWriteMask = colorWritemask;
	blendAttachmentState.blendEnable = blendEnable;
	return blendAttachmentState;
}

VkPipelineColorBlendStateCreateInfo vkTools::initializers::pipelineColorBlendStateCreateInfo(
	uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState * pAttachments)
{
	VkPipelineColorBlendStateCreateInfo   colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = NULL;
	colorBlendState.attachmentCount = attachmentCount;
	colorBlendState.pAttachments = pAttachments;
	return colorBlendState;
}

VkPipelineDepthStencilStateCreateInfo vkTools::initializers::pipelineDepthStencilStateCreateInfo(
	VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};

	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = depthTestEnable;
	depthStencilState.depthWriteEnable = depthWriteEnable;
	depthStencilState.depthCompareOp = depthCompareOp;
	//depthStencilState.depthBoundsTestEnable = VK_TRUE;
	depthStencilState.front = depthStencilState.back;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	return depthStencilState;
}

VkPipelineViewportStateCreateInfo vkTools::initializers::pipelineViewportStateCreateInfo(
	uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = viewportCount;
	viewportState.scissorCount = scissorCount;
	viewportState.flags = flags;
	return viewportState;
}

VkPipelineMultisampleStateCreateInfo vkTools::initializers::pipelineMultisampleStateCreateInfo(
	VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags)
{
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = rasterizationSamples;
	return multisampleState;
}

VkPipelineDynamicStateCreateInfo vkTools::initializers::pipelineDynamicStateCreateInfo(
	const VkDynamicState * pDynamicStates, uint32_t dynamicStateCount, VkPipelineDynamicStateCreateFlags flags)
{
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
	pipelineDynamicStateCreateInfo.sType =  VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
	return pipelineDynamicStateCreateInfo;
}

VkPipelineTessellationStateCreateInfo vkTools::initializers::pipelineTesselationStateCreateInfo(uint32_t patchControlPoints)
{
	VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo = {};
	pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
	return pipelineTessellationStateCreateInfo;
}

VkGraphicsPipelineCreateInfo vkTools::initializers::pipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags)
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = NULL;
	pipelineCreateInfo.layout = layout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.flags = flags;
	return pipelineCreateInfo;
}

VkClearAttachment vkTools::initializers::clearAttachment(VkImageAspectFlags aspectMask, uint32_t colorAttachment, VkClearValue clearValue)
{
	VkClearAttachment attachment = {};
	attachment.aspectMask = aspectMask;
	attachment.colorAttachment = colorAttachment;
	attachment.clearValue = clearValue;
	return attachment;
}

namespace vkTools {
	CShader::CShader(std::string vsPath, std::string fsPath, std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings,std::vector<VkVertexInputBindingDescription> bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescription) {
		addShader(vsPath, VK_SHADER_STAGE_VERTEX_BIT);
		addShader(fsPath, VK_SHADER_STAGE_FRAGMENT_BIT);
		m_layoutBindings = setLayoutBindings;
		m_bindingDescription = bindingDescription;
		m_attributeDescription = attributeDescription;
		m_descriptorLayoutCreateInfo = vkTools::initializers::descriptorSetLayoutCreateInfo(m_layoutBindings.data(), static_cast<uint32_t>(m_layoutBindings.size()));
		m_pipelineLayoutCreateInfo = vkTools::initializers::pipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);
		printf("DELETE ME : bl %i bd %i ad %i\n", m_layoutBindings.size(), m_bindingDescription.size(), m_attributeDescription.size());
		setInputState(static_cast<uint32_t>(m_bindingDescription.size()), m_bindingDescription.data(), static_cast<uint32_t>(m_attributeDescription.size()), (m_attributeDescription.data()));
	}

	CShader::~CShader()
	{
	}

	void CShader::load(VkDevice device) {
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &m_descriptorLayoutCreateInfo, nullptr, &m_descriptorSetLayout));
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &m_pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
		shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

		m_shaderModules.resize(m_paths.size());
		m_shaderStages.resize(m_paths.size());

		for (size_t i = 0; i < m_paths.size();i++) {
			m_shaderModules[i] = loadShader(m_paths[i].c_str(), device, m_flags[i]);
			shaderStageCreateInfo.stage = m_flags[i];
			shaderStageCreateInfo.module = m_shaderModules[i];
			shaderStageCreateInfo.pName = "main";
			assert(shaderStageCreateInfo.module != NULL);
			m_shaderModules[i] = shaderStageCreateInfo.module;

			m_shaderStages[i] = shaderStageCreateInfo;

		}
	}

	void CShader::clear(VkDevice device)
	{
		vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
		for (size_t i = 0; i < m_shaderModules.size();i++) {
			vkDestroyShaderModule(device, m_shaderModules[i], nullptr);
		}
	}

	std::vector<VkShaderModule> CShader::getShaderModules() const
	{
		return m_shaderModules;
	}

	void CShader::addShader(std::string sPath, VkShaderStageFlagBits flag) {
		m_paths.push_back(sPath);
		m_flags.push_back(flag);
	}

	void CShader::setInputState(uint32_t bindingCount ,VkVertexInputBindingDescription *bindingDescriptions , uint32_t attributeCount, VkVertexInputAttributeDescription *attributeDescriptions) {
		m_inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
		m_inputState.vertexBindingDescriptionCount = bindingCount;
		m_inputState.pVertexBindingDescriptions = bindingDescriptions;
		m_inputState.vertexAttributeDescriptionCount = attributeCount;
		m_inputState.pVertexAttributeDescriptions = attributeDescriptions;
	}

	VkPipelineLayout CShader::getPipelineLayout() const
	{
		return m_pipelineLayout;
	}

	VkPipelineVertexInputStateCreateInfo CShader::getInputState() const
	{
		return m_inputState;
	}

	std::vector<VkPipelineShaderStageCreateInfo> CShader::getShaderStages()
	{
		return m_shaderStages;
	}

	VkPipelineShaderStageCreateInfo * CShader::getShaderStagesPtr()
	{
		return m_shaderStages.data();
	}

	VkDescriptorSetLayout * CShader::getDescriptorSetLayoutPtr()
	{
		return &m_descriptorSetLayout;
	}

	VkDescriptorSet * CShader::getDescriptorSetPtr()
	{
		return m_descriptorSet;
	}

	uint32_t CShader::getDescriptorSetId() const
	{
		return m_descriptorSetId;
	}

	void CShader::attachDescriptorSet(VkDescriptorSet * descriptorSet)
	{
		m_descriptorSet = descriptorSet;
		//printf("f");
	}

	void CShader::attachDescriptorSet(uint32_t id) {
		m_descriptorSetId = id;
	}

}

bool vkTools::equal::vertexAttributeInputDescriptor(const VkVertexInputAttributeDescription & v1, const VkVertexInputAttributeDescription& v2)
{
	if (v1.binding!=v2.binding) {
		return false;
	}

	if (v1.format!=v2.format) {
		return false;
	}

	if (v1.location != v2.location) {
		return false;
	}

	if (v1.offset!=v2.offset) {
		return false;
	}

	return true;
}
