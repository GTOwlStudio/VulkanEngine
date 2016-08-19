#pragma once

#include <vulkan\vulkan.h>

class IMemoryManager
{
public:
	virtual ~IMemoryManager() {};
	virtual void requestMemory(VkDeviceSize requestSize) = 0;
	virtual VkDeviceSize requestMemorySize() const = 0;
};