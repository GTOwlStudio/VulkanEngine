#pragma once

//#include "VirtualBuffer.h"
#include <list>
#include <vector>
#include <algorithm>
#include <vulkan\vulkan.h>
#include <string>
#include "vulkanbuffer.hpp"
/*
Virtual Buffer Pool
LIMITATION : only allow to manage 4GB of memory
LIMITATION COMMENTARY : 4GB seems not much but it is a lot, cos virtual buffer
were made to allow easier Video Memory Management
NO WARRANTLY : the order of the buffer in the list is the same of the buffer in the real F**CKING WORLD, 
NO IDEA WHY THE I SET THIS WARRANTLY
*/

struct VirtualBuffer{
	VkDescriptorBufferInfo bufferInfo = {};
	//VkBufferUsageFlags usageFlags;
	size_t id; //size_t is in fact a uint64_t
	VirtualBuffer(uint64_t psize, uint64_t poffset,/* VkBufferUsageFlags flags,*/ uint64_t pid, VkBuffer buffer) :/* usageFlags(flags),*/id(pid){
		bufferInfo.range = psize;
		bufferInfo.offset = poffset;
		bufferInfo.buffer = buffer;
	}
};

class VirtualBufferPool
{
public:
	VirtualBufferPool(uint64_t poolSize, VkBufferUsageFlags flags);
	~VirtualBufferPool();
	void allocateBuffer(uint64_t* id,VkBufferUsageFlags usageFlags, uint64_t size, std::string tag, VkBuffer buffer, uint64_t offset = -1);
	void allocateBuffer(uint64_t* id,VkBufferUsageFlags usageFlags, uint64_t size, VkBuffer buffer, uint64_t offset = -1);
	void setBuffer(VkBuffer buffer);
	VirtualBuffer getVirtualBuffer(size_t id);
	VirtualBuffer* getVirtualBufferPtr(size_t id);
	size_t poolLength(); //Return the number of element of the pool
	VkDeviceSize getPoolSize(); //Return the size of the pool, in byte
	//void createBuffer(uint32_t siz);
	//bool checkOffset(uint32_t offset);
	//uint64_t getNextOffset(uint64_t bufferSize = 0, uint64_t offset = 0);
	std::string getInfo();

protected:
	uint64_t m_poolSize;
	std::vector<VirtualBuffer> m_vbuffers;
	std::vector<std::string> m_buffersTag;
	VkBufferUsageFlags m_flags;
	vk::Buffer m_buffer;

	void addVirtualBuffer(uint64_t size, uint64_t offset,VkBufferUsageFlags flags, std::string tag="");
	bool checkFlags(VkBufferUsageFlags toCheck);
	/*getNextOffset, to get the next FREE SPACE offset
	PARAM : -bufferSize is the size of the bufferRequested, fault = 0
			-offset is the start point of the research,		fault = 0
	*/
	bool checkOffset(uint64_t offset);
	uint64_t getNextOffset(uint64_t bufferSize=0, uint64_t offset = 0);
	static uint64_t nextId;
};

