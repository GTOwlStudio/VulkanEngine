#include "MemoryManager.h"



CMemoryManager::CMemoryManager()
{
}


CMemoryManager::~CMemoryManager()
{
}

VkDeviceSize CMemoryManager::requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags)
{
	m_requestedMemorySize += requestSize;
	m_memblock.push_back(requestSize);
	m_descriptions.push_back(description);
	addFlag(flags);
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

void CMemoryManager::allocateMemory()
{
	m_virtualBuffers.push_back(VirtualBufferPool(m_requestedMemorySize, m_flags));
	uint64_t offset = 0;
	for (size_t i = 0; i < m_memblock.size();i++) {
		m_virtualBuffers.back().allocateBuffer(nullptr, m_flags, m_memblock[i],offset);
		offset += m_memblock[i];
	}
	printf("[Allocatememory]%s\n",m_virtualBuffers.back().getInfo().c_str());
}

void CMemoryManager::addFlag(VkBufferUsageFlags flags)
{
	if ((m_flags&flags)==0) {
		m_flags |= flags;
	}
}
