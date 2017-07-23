#pragma once

#include <vulkan\vulkan.h>
#include <string>
#include "VirtualBufferPool.h"


class IMemoryManager
{
public:
	virtual ~IMemoryManager() {};
	virtual size_t requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT) = 0;
	//virtual size_t requestUniformBuffer(VkDeviceSize uboSize, std::string description) = 0;
	//virtual void requestMemory(VkDeviceSize requestSize, std::string description, VkDeviceSize *bOffset, VkBuffer *buffer) = 0;
	
	virtual void init_data(VkVertexInputAttributeDescription id, void* data, uint64_t data_size) = 0;

	virtual size_t add_temporary_mem_block() = 0;


	virtual VkDeviceSize requestedMemorySize() const = 0;
	virtual std::vector<VkDeviceSize> requestedBuffers() = 0;
	virtual std::string getGlobalMemoryDescription(std::string separator=" | ") = 0;
	virtual void allocateMemory() = 0;
	virtual VirtualBuffer getVirtualBuffer(uint64_t id) = 0;
	virtual VirtualBuffer* getVirtualBufferPtr(uint64_t id) = 0;
	virtual VkBufferUsageFlags getFlags() = 0;

	virtual uint64_t getUniformRealBufferId() = 0;
	virtual uint64_t getUniformBufferId(uint64_t id) = 0;
	//virtual uint64_t getUniformBufferVirtualId() = 0;

};