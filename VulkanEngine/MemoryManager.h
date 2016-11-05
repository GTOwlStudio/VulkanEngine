#pragma once
#include "IMemoryManager.h"
#include <vector>
class CMemoryManager : public IMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
	virtual VkDeviceSize requestMemory(VkDeviceSize requestSize, std::string description);
	virtual VkDeviceSize requestedMemorySize() const;
	virtual std::string getGlobalMemoryDescription(std::string separator = " | ");

protected:
	VkDeviceSize m_requestedMemorySize = 0;
	std::vector<VkDeviceSize> m_memblock;
	std::vector<std::string> m_descriptions;

};

