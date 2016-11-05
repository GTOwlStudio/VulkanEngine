#include "MemoryManager.h"



CMemoryManager::CMemoryManager()
{
}


CMemoryManager::~CMemoryManager()
{
}

void CMemoryManager::allocBuffer(uint32_t size, VkBufferUsageFlags flags)
{
	Buffer b(size, flags);
	buffers.push_back(b);
	buffers.back().id=buffers.size()-1;
}

void CMemoryManager::displayBuffers()
{
	for (size_t i = 0; i < buffers.size();i++) {
		printf("%i\n%s\n",buffers[i].size,helper::flagsToString(buffers[i].flags, "\t","\n").c_str());
	}
}
