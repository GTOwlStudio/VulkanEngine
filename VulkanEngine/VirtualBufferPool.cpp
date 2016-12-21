#include "VirtualBufferPool.h"

uint64_t VirtualBufferPool::nextId = 0;


VirtualBufferPool::VirtualBufferPool(uint64_t poolSize, VkBufferUsageFlags flags) : m_poolSize(poolSize), m_flags(flags)
{
}


VirtualBufferPool::~VirtualBufferPool()
{
}



void VirtualBufferPool::allocateBuffer(uint64_t* id, VkBufferUsageFlags usageFlags, uint64_t size, std::string tag, uint64_t offset)
{
	if (!checkFlags(usageFlags)) {
		printf("[ERROR]Buffer flags not compatible with the pool, buffer not created\n");
		if (id!=nullptr) { *id = -1; }
		return;
	}
	uint64_t finalOffset = offset;
	if (m_vbuffers.empty()) {
		finalOffset = 0;
	}
	if (finalOffset == -1) {
		finalOffset = getNextOffset();
		if (finalOffset == -1) {
			if (m_vbuffers.back().offset + m_vbuffers.back().size + size<m_poolSize) {
				finalOffset = m_vbuffers.back().offset + m_vbuffers.back().size;
			}
			else {
				printf("[ERROR]No more FREE space\n");
				if (id != nullptr) { *id = -1; }
				return;
			}
		}
	}
	if ((offset + size > m_poolSize)) {
		printf("[ERROR]Buffer out of range so not created\n");
		if (id!=nullptr) { *id = -1; }
		return;
	}
	if (id != nullptr) { *id = nextId; }
	;
	addVirtualBuffer(size, finalOffset, usageFlags, tag);
}

void VirtualBufferPool::allocateBuffer(uint64_t* id, VkBufferUsageFlags usageFlags, uint64_t size, uint64_t offset)
{
	allocateBuffer(id, usageFlags, size,"",offset);
}

std::string VirtualBufferPool::getInfo()
{
	std::string info = "VirtualBufferPoolSize = " + std::to_string(m_poolSize) + "\n";
	//info += "VirtualBufferPoolSize = " + std::to_string(m_poolSize);
	std::vector<std::string> allStr;

	for (std::string st : m_buffersTag) {
		allStr.push_back(st);
	}
	int i = 0;
	for (VirtualBuffer vbs : m_vbuffers) {
		info += std::to_string(vbs.offset) + "(";;
		info += std::to_string(vbs.size);
		if (allStr[i]!="") {
			info += ", ";
			info += allStr[i];
		}
		info += ",id=";
		info += std::to_string(vbs.id);
		info += ") ";
		i += 1;
	}

	

	return info;
}

void VirtualBufferPool::addVirtualBuffer(uint64_t size, uint64_t offset,VkBufferUsageFlags flags, std::string tag)
{
	m_vbuffers.push_back(VirtualBuffer(size,offset,/*flags,*/ VirtualBufferPool::nextId));
	m_buffersTag.push_back(tag);
	VirtualBufferPool::nextId += 1;
}

bool VirtualBufferPool::checkFlags(VkBufferUsageFlags toCheck)
{
	if ((toCheck&m_flags)==toCheck) {
		return true;
	}
	return false;
}

bool VirtualBufferPool::checkOffset(uint64_t offset)
{
	uint64_t tmp = 0;
	for (std::list<VirtualBuffer>::const_iterator i = m_vbuffers.begin(); i != m_vbuffers.end();i++) {
		if ((offset>i->offset)&&(offset<i->offset+i->size)) {
			return false;
		}
	}
	return true;
}

uint64_t VirtualBufferPool::getNextOffset(uint64_t bufferSize, uint64_t offset)
{
	if (bufferSize + offset > m_poolSize) {
		return -1;
	}
	uint64_t noffset = -1;
	std::list<VirtualBuffer>::const_iterator iminus = m_vbuffers.begin();
	std::list<VirtualBuffer>::const_iterator it = m_vbuffers.begin();
	++it;
	uint64_t space;
	while (it != m_vbuffers.end()) {

		space =  it->offset - (iminus->offset + iminus->size);
		if ((space>0)&&(space>=bufferSize)) {
			noffset = std::min<uint64_t>(noffset,it->offset-space);
		}
		printf("[SPACE DEBUG]%i\n", space);
		++it;
		++iminus;
	}

	return noffset;
}

