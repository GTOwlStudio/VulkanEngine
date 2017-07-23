#include "MemoryManager.h"



CMemoryManager::CMemoryManager() : m_sortString("")
{
}


CMemoryManager::~CMemoryManager()
{
	deleteInitData();
}

size_t CMemoryManager::requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags)
{
	
	if (flags != VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
		addFlag(flags);
		m_requestedMemorySize += requestSize;
		m_memblock.push_back(requestSize);
		m_descriptions.push_back(description);
		return m_memblock.size() - 1;//m_requestedMemorySize - requestSize;
	}
	else {
		m_uniformBufferInfo.offsets.push_back(m_uniformBufferInfo.bufferSize);
		m_uniformBufferInfo.bufferSize += requestSize;
		m_uniformBufferInfo.sizes.push_back(requestSize);	
		return m_uniformBufferInfo.sizes.size() - 1;
	}
	
	return 0;
	
}



void CMemoryManager::init_data(VkVertexInputAttributeDescription id, void * data, uint64_t data_size)
{
	size_t i = addViadId(id);
	m_init_data_adresses.push_back(new char[data_size]);
	m_init_data_sizes.push_back(data_size);
}

size_t CMemoryManager::add_temporary_mem_block()
{
	return size_t();
}

void CMemoryManager::delete_temporary_mem_block(size_t blockId)
{
}

VkDeviceSize CMemoryManager::requestedMemorySize() const
{
	return m_requestedMemorySize;
}

std::vector<VkDeviceSize> CMemoryManager::requestedBuffers()
{
	std::vector<VkDeviceSize> sizes;
	
	for (size_t i = 0; i < m_virtualBuffers.size(); i++) {
		sizes.push_back(m_virtualBuffers[i].getPoolSize());
	}

	return sizes;
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
	//std::vector<VkDeviceSize> uniformBufferAllocation;
	//#DIRTY
	m_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	m_virtualBuffers.push_back(VirtualBufferPool(m_requestedMemorySize, m_flags));
	uint64_t offset = 0;
	uint64_t testptr;
	printf("allocate data\n");
	for (size_t i = 0; i < m_memblock.size();i++) {
		m_virtualBuffers.back().allocateBuffer(&testptr, m_flags,m_memblock[i], nullptr,offset);
		printf("%i\n", testptr);
		offset += m_memblock[i];
	}
	
	gEnv->pRenderer->createBuffer(&gEnv->bbid, m_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_requestedMemorySize);
	m_virtualBuffers.back().setBuffer(gEnv->pRenderer->getBuffer(gEnv->bbid));

	m_virtualBuffers.push_back(VirtualBufferPool(m_uniformBufferInfo.bufferSize, m_uniformBufferInfo.flags));
	offset = 0;
	printf("allocate uniformBuffers\n");
	for (size_t i = 0; i < m_uniformBufferInfo.sizes.size();i++) {
		m_virtualBuffers.back().allocateBuffer(&testptr, m_uniformBufferInfo.flags, m_uniformBufferInfo.sizes[i], nullptr, m_uniformBufferInfo.offsets[i]);
		printf("%i\n", testptr);
	}

	gEnv->pRenderer->createBuffer(&m_uniformBufferInfo.bufferId, m_uniformBufferInfo.flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_uniformBufferInfo.bufferSize);
	m_virtualBuffers.back().setBuffer(gEnv->pRenderer->getBuffer(m_uniformBufferInfo.bufferId));
	

	//gEnv->pRenderer->createBuffer(, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gEnv->pMemoryManager->requestedMemorySize()
	printf("\n[Allocatememory]\n");
	for (size_t i = 0; i < m_virtualBuffers.size();i++) {
		printf("%s\n\n",m_virtualBuffers[i].getInfo().c_str());
	}
	//printf("[Allocatememory]%s\n",m_virtualBuffers.back().getInfo().c_str());
}

VirtualBuffer CMemoryManager::getVirtualBuffer(uint64_t id)
{
	size_t tmp = 0;
	size_t lastTmp = 0;
	for (size_t i = 0; i < m_virtualBuffers.size();i++) {
		lastTmp = tmp;
		tmp += m_virtualBuffers[i].poolLength();
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
		tmp += m_virtualBuffers[i].poolLength();
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

VkBufferUsageFlags CMemoryManager::getFlags()
{
	return m_flags;
}

uint64_t CMemoryManager::getUniformRealBufferId()
{
	return m_uniformBufferInfo.bufferId;
}

uint64_t CMemoryManager::getUniformBufferId(uint64_t id)
{
	uint64_t bid = 0;
	for (size_t i = 0; i < m_uniformBufferInfo.bufferId; i++) {
		bid += m_virtualBuffers[i].poolLength();
	}
	return bid;
}

uint64_t CMemoryManager::getUniformBufferVirtualId()
{
	return m_uniformBufferInfo.virtualBufferId;
}

bool CMemoryManager::viadIdExist(VkVertexInputAttributeDescription id)
{
	for (VkVertexInputAttributeDescription attrb : m_viadIds) {
		if (vkTools::equal::vertexAttributeInputDescriptor(attrb, id)) {
			return true;
		}
	}
	return false;
}

size_t CMemoryManager::viadIndice(VkVertexInputAttributeDescription id)
{
	for (size_t i = 0; i < m_viadIds.size(); i++) {
		if (vkTools::equal::vertexAttributeInputDescriptor(m_viadIds[i], id)) {
			return i;
		}
	}
	return -1;
}

size_t CMemoryManager::addViadId(VkVertexInputAttributeDescription id)
{
	size_t indice = viadIndice(id);
	if (indice != size_t(-1)) {
		return indice;
	}
	m_viadIds.push_back(id);
	return m_viadIds.size() - 1;
}

void CMemoryManager::deleteInitData()
{
	for (size_t i = 0; i < m_init_data_adresses.size(); i++){
		if (m_init_data_adresses[i]!=nullptr) {
			delete[] m_init_data_adresses[i];
		}
		m_init_data_adresses[i] = nullptr;
	}
}

void CMemoryManager::addFlag(VkBufferUsageFlags flags)
{
	if ((m_flags&flags)==0) {
		m_flags |= flags;
	}
}
