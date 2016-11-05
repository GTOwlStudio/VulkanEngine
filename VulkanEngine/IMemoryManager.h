#pragma once

#include <vulkan\vulkan.h>
#include <string>

class IMemoryManager
{
public:
	virtual ~IMemoryManager() {};
	virtual VkDeviceSize requestMemory(VkDeviceSize requestSize, std::string description) = 0;
	//virtual void requestMemory(VkDeviceSize requestSize, std::string description, VkDeviceSize *bOffset, VkBuffer *buffer) = 0;
	virtual VkDeviceSize requestedMemorySize() const = 0;
	virtual std::string getGlobalMemoryDescription(std::string separator=" | ") = 0;

};