#pragma once
#include "vulkan_header.h"
#include "IMemoryManager.h"
#include "System.h"
#include <vector>

class CSystem;

struct temporay_mem_block{
	std::vector<char*> data_adress;
	std::vector<uint64_t> data_sizes;
};

class CMemoryManager : public IMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
	virtual size_t requestMemory(VkDeviceSize requestSize, std::string description, VkBufferUsageFlags flags);
	//virtual size_t requestMemory(VkDeviceSize requestSize, std::string description,  );
	virtual void init_data(VkVertexInputAttributeDescription id, void* data, uint64_t data_size);

	virtual size_t add_temporary_mem_block();
	virtual void delete_temporary_mem_block(size_t blockId);

	virtual VkDeviceSize requestedMemorySize() const;
	virtual std::vector<VkDeviceSize> requestedBuffers();
	virtual std::string getGlobalMemoryDescription(std::string separator = " | ");
	virtual void allocateMemory();
	virtual VirtualBuffer getVirtualBuffer(uint64_t id);
	virtual VirtualBuffer* getVirtualBufferPtr(uint64_t id);
	virtual VkBufferUsageFlags getFlags();

	virtual uint64_t getUniformRealBufferId();//return the real id of the buffer
	virtual uint64_t getUniformBufferId(uint64_t id); //retrun the virtual id of the ubo
	virtual uint64_t getUniformBufferVirtualId();

protected:
	VkDeviceSize m_requestedMemorySize = 0;
	std::vector<VkDeviceSize> m_memblock;
	std::vector<std::string> m_descriptions;
	std::vector<VirtualBufferPool> m_virtualBuffers;
	VkBufferUsageFlags m_flags;

	std::vector<uint64_t> m_indices;

	std::string m_sortString;

	std::vector<VkVertexInputAttributeDescription> m_viadIds;//VertexInputAttributeDescription Ids
	bool viadIdExist(VkVertexInputAttributeDescription id);//DescriptorSetLayoutBindings Id exist ?
	size_t viadIndice(VkVertexInputAttributeDescription id); //Return the indice of the id in the m_viadIds array, return -1 if not inside
	size_t addViadId(VkVertexInputAttributeDescription id); //Add DescriptorSetLayoutBindings Id 
	
	
	std::vector<char*> m_init_data_adresses;
	std::vector<uint64_t> m_init_data_sizes;
	
	void deleteInitData();

	struct {
		uint64_t bufferId;
		uint64_t virtualBufferId;
		VkDeviceSize bufferSize = 0;
		std::vector<VkDeviceSize> offsets;
		std::vector<VkDeviceSize> sizes;
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	} m_uniformBufferInfo;


	void addFlag(VkBufferUsageFlags flags);

};

