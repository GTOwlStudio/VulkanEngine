#pragma once

#include <string>
#include <vulkan\vulkan.h>

namespace helper {
	
	std::string flagsToString(VkBufferUsageFlags flags, std::string sufix = "", std::string prefix = " | ");
	
}