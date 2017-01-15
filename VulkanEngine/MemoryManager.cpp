#include "MemoryManager.h"



CMemoryManager::CMemoryManager()
{
}


CMemoryManager::~CMemoryManager()
{
}

size_t CMemoryManager::requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags)
{
	m_requestedMemorySize += requestSize;
	m_memblock.push_back(requestSize);
	m_descriptions.push_back(description);
	addFlag(flags);
	
	return m_memblock.size()-1;//m_requestedMemorySize - requestSize;
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

void CMemoryManager::allocateMemory(VkBuffer buffer)
{
	m_virtualBuffers.push_back(VirtualBufferPool(m_requestedMemorySize, m_flags));
	uint64_t offset = 0;
	uint64_t testptr;
	printf("allocateMemory()\n");
	for (size_t i = 0; i < m_memblock.size();i++) {
		m_virtualBuffers.back().allocateBuffer(&testptr, m_flags,m_memblock[i], buffer,offset);
		printf("%i\n", testptr);
		offset += m_memblock[i];
	}
	//gEnv->pRenderer->createBuffer(, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gEnv->pMemoryManager->requestedMemorySize()

	printf("[Allocatememory]%s\n",m_virtualBuffers.back().getInfo().c_str());
}

VirtualBuffer CMemoryManager::getVirtualBuffer(uint64_t id)
{
	size_t tmp = 0;
	size_t lastTmp = 0;
	for (size_t i = 0; i < m_virtualBuffers.size();i++) {
		lastTmp = tmp;
		tmp += m_virtualBuffers[i].poolSize();
		if (id<tmp) {
			return m_virtualBuffers[i].getVirtualBuffer(id-lastTmp);
		}
		
		
	}

	assert(id < tmp);
	
	return VirtualBuffer(0, 0, 0, 0);
}

VirtualBuffer * CMemoryManager::getVirtualBufferPtr(uint64_t id)
{
	size_t tmp = 0;
	size_t lastTmp = 0;
	for (size_t i = 0; i < m_virtualBuffers.size(); i++) {
		lastTmp = tmp;
		tmp += m_virtualBuffers[i].poolSize();
		if (id<tmp) {
			//size_t tt = id - lastTmp;
			//VirrtualBuffer buf = &m_virtualBuffers();
			//return &m_virtualBuffers[i].getVirtualBuffer(tt);
			return m_virtualBuffers[i].getVirtualBufferPtr(id - lastTmp);
		}
	}

	assert(id < tmp);

	return nullptr;
}

void CMemoryManager::addFlag(VkBufferUsageFlags flags)
{
	if ((m_flags&flags)==0) {
		m_flags |= flags;
	}
}
