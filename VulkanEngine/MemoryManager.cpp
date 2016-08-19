#include "MemoryManager.h"



CMemoryManager::CMemoryManager()
{
}


CMemoryManager::~CMemoryManager()
{
}

void CMemoryManager::requestMemory(VkDeviceSize requestSize)
{
	m_requestMemorySize += requestSize;
}

VkDeviceSize CMemoryManager::requestMemorySize() const
{
	return m_requestMemorySize;
}
