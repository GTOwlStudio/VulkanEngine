#pragma once

#include <assert.h>
#include <vulkan\vulkan.h>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <string>

namespace vkDebug
{
	extern int validationLayerCount;
	extern const char *validationLayerNames[];

	VkBool32 messageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData);

	void setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callback);
	void setupDebugMarker(VkDevice device);
	void freeDebugCallback(VkInstance instance);
	
	void setObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char *name);

	namespace DebugMarker {

	}

	void arrayToFile(void* data, uint32_t size, std::string filename, bool append = false, bool newLine = false);
	void stringToFile(std::string s, std::string filename, bool append = false);
}