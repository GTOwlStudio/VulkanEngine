#pragma once

#include <vulkan\vulkan.h>
#include <string>
#include "VirtualBufferPool.h"


class IMemoryManager
{
public:
	virtual ~IMemoryManager() {};
	virtual size_t requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT) = 0;
	//virtual void requestMemory(VkDeviceSize requestSize, std::string description, VkDeviceSize *bOffset, VkBuffer *buffer) = 0;
	virtual VkDeviceSize requestedMemorySize() const = 0;
	virtual std::string getGlobalMemoryDescription(std::string separator=" | ") = 0;
	virtual void allocateMemory(VkBuffer buffer) = 0;
	virtual VirtualBuffer getVirtualBuffer(uint64_t id) = 0;
	virtual VirtualBuffer* getVirtualBufferPtr(uint64_t id) = 0;

};