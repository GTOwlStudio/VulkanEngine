#include "vulkandebug.h"
#include <iostream>


namespace vkDebug
{
	int validationLayerCount = 1;
	const char *validationLayerNames[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
	PFN_vkDebugReportMessageEXT dbgBreakCallback;

	PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjetTag = VK_NULL_HANDLE;
	PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd = VK_NULL_HANDLE;
	PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert = VK_NULL_HANDLE;

	VkDebugReportCallbackEXT msgCallback;

	VkBool32 messageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData)
	{
		char *message = (char *)malloc(strlen(pMsg) + 100);

		assert(message);

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			std::cout << "ERROR: " << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << "\n";
		}
		else
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			{
				// Uncomment to see warnings
				std::cout << "WARNING: " << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << "\n";
			}
			else
			{
				return false;
			}

		fflush(stdout);

		free(message);
		return false;
	}

	void setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
	{
		CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
		DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		dbgBreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
		dbgCreateInfo.flags = flags;

		VkResult err = CreateDebugReportCallback(
			instance,
			&dbgCreateInfo,
			nullptr,
			&msgCallback);
		assert(!err);
	}

	void setupDebugMarker(VkDevice device)
	{
		pfnDebugMarkerSetObjetTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
		pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
		pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
		pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
		pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	}

	void freeDebugCallback(VkInstance instance) {
		/*if (msgCallback !=VK_NULL_HANDLE) {
		TODO
		}*/
	}
	void setObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char *name)
	{
		// Check for a valid function pointer
		if (pfnDebugMarkerSetObjectName)
		{
			VkDebugMarkerObjectNameInfoEXT nameInfo = {};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = objectType;
			nameInfo.object = object;
			nameInfo.pObjectName = name;
			pfnDebugMarkerSetObjectName(device, &nameInfo);
		}
	}

	void arrayToFile(void * data, uint32_t size, std::string filename, bool append, bool newLine)
	{
	//#simp if
		std::ofstream file;
		if (append) {
			file.open(filename.c_str(),  std::ios::app);
		}

		else {
			file.open(filename.c_str());
		}
		

		

		if (!file) {
			printf("ERROR : Cannot open file %s\n", filename.c_str());
			return;
		}

		file.write((char*)data, size);
		//file.write("\0", sizeof("\0"));

		if (newLine) {
			file << std::endl;
		}

		file.close();


	}

	void stringToFile(std::string s, std::string filename, bool append)
	{

		std::ofstream file;

		if (append) {
			file.open(filename.c_str(), /*std::ios::binary |*/ std::ios::app);
		}

		else {
			file.open(filename.c_str()/*, std::ios::binary*/);
		}


		if (!file) {
			printf("ERROR : Cannot open file %s\n", filename.c_str());
			return;
		}

		file << s.c_str();

		file.close();

	}


	
	
}
