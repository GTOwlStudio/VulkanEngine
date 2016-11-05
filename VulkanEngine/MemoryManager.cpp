#include "MemoryManager.h"



CMemoryManager::CMemoryManager()
{
}


CMemoryManager::~CMemoryManager()
{
}

VkDeviceSize CMemoryManager::requestMemory(VkDeviceSize requestSize, std::string description)
{
	m_requestedMemorySize += requestSize;
	m_memblock.push_back(requestSize);
	m_descriptions.push_back(description);
	return m_requestedMemorySize - requestSize;
}

VkDeviceSize CMemoryManager::requestedMemorySize() const
{
	return m_requestedMemorySize;
}

std::string CMemoryManager::getGlobalMemoryDescription(std::string separator)
{
	std::string gd = "";//Global Description
	for (size_t i = 0; i < m_descriptions.size();i++) {
		gd += m_descriptions[i] + separator;
	}
	return gd;
}
