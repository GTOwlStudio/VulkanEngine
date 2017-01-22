#pragma once
#include "vulkan_header.h"
#include "IMemoryManager.h"
#include "System.h"
#include <vector>

class CSystem;

class CMemoryManager : public IMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
	virtual size_t requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags);
	virtual VkDeviceSize requestedMemorySize() const;
	virtual std::vector<VkDeviceSize> requestedBuffers();
	virtual std::string getGlobalMemoryDescription(std::string separator = " | ");
	virtual void allocateMemory();
	virtual VirtualBuffer getVirtualBuffer(uint64_t id);
	virtual VirtualBuffer* getVirtualBufferPtr(uint64_t id);
	virtual VkBufferUsageFlags getFlags();

protected:
	VkDeviceSize m_requestedMemorySize = 0;
	std::vector<VkDeviceSize> m_memblock;
	std::vector<std::string> m_descriptions;
	std::vector<VirtualBufferPool> m_virtualBuffers;
	VkBufferUsageFlags m_flags;

	struct {
		uint64_t bufferId;
		VkDeviceSize bufferSize = 0;
		std::vector<VkDeviceSize> offsets;
		std::vector<VkDeviceSize> sizes;
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	} m_uniformBufferInfo;


	void addFlag(VkBufferUsageFlags flags);

};

