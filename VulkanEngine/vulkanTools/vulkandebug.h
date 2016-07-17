#pragma once

#include <assert.h>
#include <vulkan\vulkan.h>

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
	void freeDebugCallback(VkInstance instance);
}