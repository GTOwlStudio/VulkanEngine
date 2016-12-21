#pragma once
#include "IMemoryManager.h"
#include <vector>
#include "VirtualBufferPool.h"

class CMemoryManager : public IMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
	virtual VkDeviceSize requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags);
	virtual VkDeviceSize requestedMemorySize() const;
	virtual std::string getGlobalMemoryDescription(std::string separator = " | ");
	virtual void allocateMemory();
	

protected:
	VkDeviceSize m_requestedMemorySize = 0;
	std::vector<VkDeviceSize> m_memblock;
	std::vector<std::string> m_descriptions;
	std::vector<VirtualBufferPool> m_virtualBuffers;
	VkBufferUsageFlags m_flags;

	void addFlag(VkBufferUsageFlags flags);

};

