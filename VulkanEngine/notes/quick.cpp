#include "/VulkanSDK/1.0.17.0/Include/vulkan/vulkan.h"
#include <vector>

struct DrawIndexedObject
{
	VkDescriptorSet* descriptorSets; //explicit
	VkPipelineLayout* pipelineLayout;
	VkPipeline* pipeline;
	VkBuffer* vertexBuffer; //The buffer where the data are located
	VkDeviceSize* pVertexOffset; //The offset inside the buffer where the data are located
	VkBuffer* indexBuffer; //The buffer where the indices are located (it CAN be the same as the vertexBuffer)
	VkDeviceSize indexOffset; //The offset inside the buffer where the indices are located
	uint32_t indexCount; //is the number of vertices to draw.
	uint32_t instanceCount; //is the number of instances to draw.
	uint32_t firstIndex; //is the base index within the index buffer.
	uint32_t vertexOffset; //is the value added to the vertex index before indexing into the vertex buffer.
	uint32_t firstInstance; //It's the instance ID of the first instance to draw.
} DrawIndexedObject;

std::vector<DrawIndexedObject> m_drawObjects;


void buildCommandBuffer()
{
	for (size_t i = 0; i < m_drawObjects.size();i++) {
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			&m_drawObjects[i].pipelineLayout, 0, 1, m_drawObjects[i].descriptorSets, 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &m_drawObjects[i].pipeline);

		vkCmdBindVertexBuffers(cmdBuffer, ?, 1, m_drawObjects[i].vertexBuffer, m_drawObjects[i].pVertexOffset);
		vkCmdBindIndexBuffer(cmdBuffer, m_drawObjects[i].indexBuffer, m_drawObjects[i].indexOffset, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer, m_drawObjects[i].indexCount, m_drawObjects[i].instanceCount, m_drawObjects[i].firstIndex, m_drawObjects[i].vertexOffset, m_drawObjects[i].firstInstance);

	}
}