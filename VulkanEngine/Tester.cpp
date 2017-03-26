#include "Tester.h"



CTester::CTester()
{
	m_draw.bufferId = gEnv->pMemoryManager->requestMemory((sizeof(VertexC)*4) + (sizeof(uint32_t)*meshhelper::QUAD_INDICES_COUNT), "tester");
	m_draw.UBO.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());
	m_draw.UBO_bufferId = gEnv->pMemoryManager->requestMemory(sizeof(m_draw.UBO), "gUBO", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	m_draw.descriptorSetId = gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,"color");

}


CTester::~CTester()
{
}

void CTester::load() {

	gEnv->pRenderer->addRenderPass("tester", VK_ATTACHMENT_LOAD_OP_LOAD);

	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass("tester"), "tester", false);
	loadGraphics();
	updateUniformBuffer();
}

void CTester::updateUniformBuffer()
{
	//m_draw.UBO.projection = glm::ortho()
	gEnv->pRenderer->bufferSubData(gEnv->pMemoryManager->getUniformRealBufferId(), sizeof(m_draw.UBO), m_draw.UBO_bufferId, &m_draw.UBO);
}

void CTester::loadGraphics()
{
	VertexC* vertices = new VertexC[meshhelper::QUAD_VERTICES_COUNT];
	uint32_t* indices = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
	meshhelper::quadVertices(50, 50, 200, 200, 0.25f, glm::vec3(1.0f, 0.0f, 1.0f), vertices);
	meshhelper::quadIndices(indices);
	
	gEnv->pRenderer->bufferSubData(m_draw.bufferId, sizeof(VertexC)*4, 0, vertices);
	gEnv->pRenderer->bufferSubData(m_draw.bufferId, sizeof(uint32_t) * 6, sizeof(VertexC) * 4, indices);

	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0),
		gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId);

	std::vector<VkWriteDescriptorSet> writeDescriptors;
	writeDescriptors.push_back(vkTools::initializers::writeDescriptorSet(
		*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId),
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
		0,
		&gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));

	gEnv->pRenderer->addWriteDescriptorSet(writeDescriptors);
	gEnv->pRenderer->updateDescriptorSets();

	delete[] vertices;
	delete[] indices;

	std::vector<SIndexedDrawInfo> drawInfo;
	drawInfo.resize(2);
	m_draw.gOffsets[0] = 0;
	drawInfo[0].bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId));
	drawInfo[0].bindPipeline(gEnv->pRenderer->getPipeline("tester"));
	//drawInfo[j].bindVertexBuffers(buffers[groups[j].ids[0]], m_draw.gOffset.size(), m_draw.gOffset.data());
	drawInfo[0].bindVertexBuffers(gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.bufferId)->bufferInfo.buffer, 1, m_draw.gOffsets);
	drawInfo[0].bindIndexBuffer(gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.bufferId)->bufferInfo.buffer, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.bufferId)->bufferInfo.offset + sizeof(VertexC) * 4, VK_INDEX_TYPE_UINT32);
	//drawInfo[j].drawIndexed(indices.size(), 1, 0, 0, 0);//#enchancement for better flexibility
	//drawInfo[j].drawIndexed(groups[j].indicesCount, 1, 0, 0, 0);//#enchancement for better flexibility #done
	drawInfo[0].drawIndexed(6, 1,0, 0, 0);//#enchancement for better flexibility #done
																					  //drawInfo[groups[j].ids[0]];
																					  //gEnv->pRenderer->addIndexedDraw(drawInfo[groups[j].ids[0]], gEnv->pRenderer->getRenderPass(m_renderPassName),"gui"); //#warninig unflexibility with m_renderPassName

	gEnv->pRenderer->addIndexedDraw(drawInfo[0], gEnv->pRenderer->getRenderPass("tester"), "tester"); //#warninig unflexibility with m_renderPassName
																										   //}

}
