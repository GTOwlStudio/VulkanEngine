#pragma once
#include "IMemoryManager.h"
class CMemoryManager : public IMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
	virtual void requestMemory(VkDeviceSize requestSize);
	virtual VkDeviceSize requestMemorySize() const;

protected:
	VkDeviceSize m_requestMemorySize = 0;

};

