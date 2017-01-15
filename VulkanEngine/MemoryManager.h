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
	virtual std::string getGlobalMemoryDescription(std::string separator = " | ");
	virtual void allocateMemory(VkBuffer buffer);
	virtual VirtualBuffer getVirtualBuffer(uint64_t id);
	virtual VirtualBuffer* getVirtualBufferPtr(uint64_t id);

protected:
	VkDeviceSize m_requestedMemorySize = 0;
	std::vector<VkDeviceSize> m_memblock;
	std::vector<std::string> m_descriptions;
	std::vector<VirtualBufferPool> m_virtualBuffers;
	VkBufferUsageFlags m_flags;

	void addFlag(VkBufferUsageFlags flags);

};

