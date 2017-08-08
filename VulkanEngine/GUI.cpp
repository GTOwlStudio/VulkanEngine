#include "GUI.h"
#include "Framebuffer.h"


uint32_t GUI::instance = 0;


GUI::GUI(std::string file) : m_file(file)
{
	if (instance >= 1) {
		assert("Multiple GUI created, must only have one");
	}
	instance += 1;

	loadCreator();
	loadGuiSettings("GuiSettings.xml");
	loadFromXml(m_file);
	reorderWidgets();

	uint32_t buffer_clr_id = -1;
	uint32_t buffer_tex_id = -1;

	uint32_t clrbuf_size = 0;
	uint32_t texbuf_size = 0;

	
	std::vector<std::string> clrShaderedWidgets = { "Panel" }; //TexturedWidgets = !clrShaderedWidgets
	

	//Allocate widgets memory space
	for (size_t i = 0; i < m_widgets.size(); i++) {
		m_widgets[i]->setBufferId(gEnv->pMemoryManager->requestMemory(m_widgets[i]->gDataSize(), m_widgets[i]->getName()));
	}

	printf("clr %i tex %i idx %i\n", m_draw.vertTexSize, m_draw.vertClrSize);

	//m_draw.vertClrId = gEnv->pMemoryManager->requestMemory(m_draw.vertClrSize, "gui_color");
	//m_draw.vertTexId = gEnv->pMemoryManager->requestMemory(m_draw.vertTexSize, "gui_tex");

	m_draw.indicesOffsetId = gEnv->pMemoryManager->requestMemory(m_draw.indicesSize, "gui_idx");

	//addWidget(new Label("Sample", offset2D(50,50)));

	//addWidget(new Panel("Panel", rect2D(100, 100, 100, 100)));-
	//float matprojpar[4] = {0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight()};
	m_draw.UBO.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());
	//m_draw.UBO.projection = mat4(1.0f);
	//m_draw.UBO.projection = glm::ortho<float>(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());//GUI coherent 
	m_draw.UBO_bufferId = gEnv->pMemoryManager->requestMemory(sizeof(m_draw.UBO), "gUBO", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	printf("GUI requestedDescriptorSet\n");
	m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, "color"));
	//m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(std::vector<VkDescriptorType>{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, "tex"));
	m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(std::vector<VkDescriptorType>{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, "font"));
	//m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, "texture"));
	m_draw.descriptorSetTypes.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_draw.descriptorSetTypes.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}


GUI::~GUI()
{
	for (Widget* w : m_widgets) {
		delete w;
		w = 0;
	}
		
}

void GUI::load()
{
	if (m_renderPassName == "gui") {
		gEnv->pRenderer->addRenderPass("gui", VK_ATTACHMENT_LOAD_OP_LOAD);
	}
	
	/*m_draw.offscreen = gEnv->pRenderer->addOffscreen("gui");
	m_draw.offscreen->load(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), gEnv->pRenderer->getRenderPass(m_renderPassName));
	*///gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass("gui"), "gui");
	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("tex"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_tex", true);
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("font"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_font", true);
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_col", true);
	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("font"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_te", true);
	printf("GUI renderPass and Graphics Pipeline Created");
	printf("%i object to be load\n", static_cast<int>(m_widgets.size()));
	loadWidgets_dev();
	updateUniformBuffer();	
}

void GUI::update()
{
}

void GUI::updateUniformBuffer()
{
	//m_draw.UBO.projection = glm::ortho(0.0f, static_cast<float>(gEnv->pSystem->getWidth()), static_cast<float>(gEnv->pSystem->getHeight()), 0.0f);
	m_draw.UBO.projection = glm::ortho(0.0f,static_cast<float>(gEnv->pSystem->getWidth()), 0.0f, static_cast<float>(gEnv->pSystem->getHeight()));
	//m_draw.UBO.projection = glm::ortho(0.0f, static_cast<float>(gEnv->pSystem->getWidth()), static_cast<float>(gEnv->pSystem->getHeight()), 0.0f);
	//m_draw.UBO.projection = glm::mat4(1.0f);

	gEnv->pRenderer->bufferSubData(gEnv->pMemoryManager->getUniformRealBufferId(), sizeof(m_draw.UBO), 0, &m_draw.UBO);

}

void GUI::addWidget(Widget * widget)
{
	m_widgets.push_back(widget);
}

void GUI::getSettings(std::string sName)
{
}

std::string GUI::getNextName(std::string prefix)
{
	return prefix + std::to_string(m_widgets.size());
}

offset2D GUI::getNextPosition()
{
	offset2D np;
	for (size_t i = 0; i < m_widgets.size();i++) {
		np.x = m_widgets[i]->getBoundary().offset.x + m_widgets[i]->getBoundary().extent.width;
		if (static_cast<uint32_t>(np.x)>=gEnv->pSystem->getWidth()) {
			np.x = 0;
			np.y = m_widgets[i]->getBoundary().offset.y + m_widgets[i]->getBoundary().extent.height;
		}
	}
	return np;
}

void GUI::loadCreator()
{
	//addCreator("Panel", &GUI::creator_Panel);
	addCreator("Panel", &GUI::creator_Panel);
	addCreator("Label", &GUI::creator_guilabel);
	addCreator("Menu", &GUI::creator_menu);
	//addCreator("Panel", std::bind(&GUI::creator_Panel, this, std::placeholders::_1));
	//addCreator("Panel", [](mxml_node_t *t) {printf("PanelConstructor %s\n"); });
}

void GUI::loadFromXml(std::string file)
{
	XMLParser parser(file);

	double top = static_cast<double>(gEnv->pSystem->getHeight());
	mxml_node_t* node = parser.getTop();
	while (node!=nullptr) {
		searchAndExecute(node);
		node = parser.getNextElement(node);
	}

}

void GUI::loadGuiSettings(std::string file)
{
	XMLParser parser(file);
	std::vector<std::string> doubleSettings = {"DefaultMenuBarWidth", "DefaultHorizontalSpacerSize", "DefaultVerticalSpacerSize"};
	std::string tmp = parser.getValue("DefaultMenuBarWidth", "value");
	for (size_t i = 0; i < doubleSettings.size();i++) {
		tmp = parser.getValue(doubleSettings[i], "value");
		if (tmp != "empty:string") {
			addDoubleSetting(doubleSettings[i], std::stod(tmp));
		}
	}
}

void GUI::loadWidgets_dev() {
	VirtualBuffer *tmpBuffer = nullptr;

	size_t tmpOffset;
	std::vector<std::string> shaderNames;
	std::vector<uint32_t> descriptorSets;
	std::vector<std::string> pipelineNames;
	std::vector<VkBuffer> buffers;
	uint32_t vertexc_widgetCount = 0;
	uint32_t vertexc_start;
	uint32_t vertexc_end;
	uint32_t vertext_widgetCount = 0;
	uint32_t vertext_start;
	uint32_t vertext_end;
	std::vector<uint32_t> tmpIndices;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indicesSizes;
	std::vector<uint32_t> indicesOffsets; //The offset of the highset value use by the "pack" of indices, [0,1,2,0,2,3] 3 is indexOffset of this list
	std::vector<uint32_t> indicesPos;

	std::vector<uint32_t> rawIndices;

	uint32_t tmpIndicesOffset = 0;
	uint32_t tmpIndicesPos = 0;
	bool descriptorColorSetCreated = false;
	bool descriptorTexSetCreated = false;

	size_t tmp2ndPartPos = -1;

	for (size_t i = 0; i < m_widgets.size(); i++) {
		if (m_widgets[i]->getClassName()=="Panel") {
			printf("Panel");
			Widget* tw = m_widgets[i]; //tmp widget

			VertexC* tmpV = new VertexC[meshhelper::QUAD_VERTICES_COUNT];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
			//printf("HERE BITCH %i\n", tw->gDataSize());
			tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			
			tw->gData(tmpV);
			tw->gIndices(tmpI);

			tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(tw->getBufferId()).bufferInfo.offset;

			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);


			for (size_t k = 0; k < meshhelper::QUAD_INDICES_COUNT; k++) {
				tmpIndices.push_back(tmpI[k]/*+tmpIndicesOffset*/);
			}

			indicesPos.push_back(tmpIndicesPos);
			tmpIndicesPos += 6;
			indicesSizes.push_back(6);
			indicesOffsets.push_back(4);
			tmpIndicesOffset += 4;

			std::vector<VkWriteDescriptorSet> writeDescriptor;

			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, 1);
			VirtualBuffer* tmp_vb = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			VkDescriptorBufferInfo* tmp_descri = &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo;

			vertexc_widgetCount += 1;

			descriptorSets.push_back(m_draw.descriptorSetId[0]);
			shaderNames.push_back("color");
			pipelineNames.push_back("gui_col");
			buffers.push_back(tmpBuffer->bufferInfo.buffer);
	
			delete[] tmpV;
			delete[] tmpI;

		}

		if (m_widgets[i]->getClassName() == "guilabel") {
			printf("GuiLabel\n");
			if (tmp2ndPartPos == -1) { tmp2ndPartPos = tmpIndices.size(); }
			Widget* tw = m_widgets[i];

			size_t woTextSize = tw->getText().size() - helper::countCharacter(tw->getText(), ' ');//text size without blank
			VertexT* tmpV = new VertexT[meshhelper::QUAD_VERTICES_COUNT*woTextSize];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT*woTextSize];

			tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());

			tw->gData(tmpV);
			tw->gIndices(tmpI);

			tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(tw->getBufferId()).bufferInfo.offset;

			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);

			for (size_t k = 0; k < meshhelper::QUAD_INDICES_COUNT*woTextSize; k++) {
				tmpIndices.push_back(tmpI[k] /*+ tmpIndicesOffset*/);
			}
			indicesPos.push_back(tmpIndicesPos);
			tmpIndicesPos += 6 * woTextSize;
			indicesSizes.push_back(6 * woTextSize);
			indicesOffsets.push_back(4 * woTextSize);
			tmpIndicesOffset += 4 * woTextSize;
			vertexc_widgetCount += 1;

			descriptorSets.push_back(m_draw.descriptorSetId[1]);
			shaderNames.push_back("font");
			pipelineNames.push_back("gui_font");
			buffers.push_back(tmpBuffer->bufferInfo.buffer);

			delete[] tmpV;
			delete[] tmpI;
		}
	}

	loadDescriptorSets();

	std::vector<uint32_t> groupMask;
	struct groupId {
		std::string shader;
		std::string pipeline;
		uint32_t descriptorSetId;
		std::vector<uint32_t> ids;
		uint32_t indicesCount = 0; //Number of indices 
		uint32_t firstIndexPos = 0; //The position of the first index in the indices list (the one send to the GPU)
		uint32_t lastIndexOffset = 0; //The offset you need to add to the "raw" indices
									  //VkBuffer buffer;
		bool equal(groupId g) {
			if (g.shader != shader) {
				return false;
			}
			if (g.pipeline != pipeline) {
				return false;
			}
			if (g.descriptorSetId != descriptorSetId) {
				return false;
			}
			return true;
		}
	};

	std::vector<groupId> groups;
	groupId tmpGroup;
	bool tmpDone;
	uint32_t tj;
	size_t pos = 0;
	size_t off = 0;
	size_t mode = 0;
	indices.resize(tmpIndices.size());
	printf("\n");
	for (size_t i = 0; i < indicesSizes.size(); i++) {
		
		//off = indicesPos[i];
		if (indicesPos[i]/* + indicesSizes[i]*/ == tmp2ndPartPos && mode == 0) {
			mode = indicesPos[i];
			off = 0;
		}
		for (size_t j = 0; j < indicesSizes[i]; j++) {
			pos = indicesPos[i] + j;
			indices[pos] = tmpIndices[pos] + off/*-mode*/;
			//printf("%i %i\n",tmpIndices[pos], indices[pos]);
		}
		off += 4 * (indicesSizes[i] / 6);
		
	}

	
	gEnv->pRenderer->bufferSubData(gEnv->bbid, m_draw.indicesSize, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.offset, indices.data());
	std::vector<SIndexedDrawInfo> drawInfo;
	//tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())
	std::vector<uint32_t> usefulId = {0, static_cast<uint32_t>(tmp2ndPartPos/6)};
	std::vector<uint32_t> idx_size = {static_cast<uint32_t>(tmp2ndPartPos), static_cast<uint32_t>(indices.size() - tmp2ndPartPos)};
	std::vector<uint32_t> idx_off = { 0, static_cast<uint32_t>(indices.size() - tmp2ndPartPos) };
	std::vector<VirtualBuffer*> bufs = { gEnv->pMemoryManager->getVirtualBufferPtr(m_widgets[0]->getBufferId()), gEnv->pMemoryManager->getVirtualBufferPtr(m_widgets[(tmp2ndPartPos / 6)]->getBufferId()) };

	drawInfo.resize(2);
	
	for (size_t j = 0; j < 2; j++) {
		//VirtualBuffer* buf = gEnv->pMemoryManager->getVirtualBufferPtr(m_widgets[j*(tmp2ndPartPos / 6)]->getBufferId());
		VirtualBuffer* buf = bufs[j];
		drawInfo[j].bindDescriptorSets(gEnv->pRenderer->getShader(shaderNames[usefulId[j]])->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(descriptorSets[usefulId[j]]));
		drawInfo[j].bindPipeline(gEnv->pRenderer->getPipeline(pipelineNames[usefulId[j]]));
		drawInfo[j].bindVertexBuffers(buf->bufferInfo.buffer, 1, &buf->bufferInfo.offset);
		drawInfo[j].bindIndexBuffer(gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.buffer, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.offset, VK_INDEX_TYPE_UINT32);
		drawInfo[j].drawIndexed(idx_size[j], 1, j*tmp2ndPartPos, 0, 0);
		//drawInfo[j].drawIndexed(idx_size[j], 1, groups[j].firstIndexPos, 0, 0);//#enchancement for better flexibility #done
																						  //drawInfo[groups[j].ids[0]];
																						  //gEnv->pRenderer->addIndexedDraw(drawInfo[groups[j].ids[0]], gEnv->pRenderer->getRenderPass(m_renderPassName),"gui"); //#warninig unflexibility with m_renderPassName
		
		gEnv->pRenderer->addIndexedDraw(drawInfo[j], gEnv->pRenderer->getRenderPass(m_renderPassName), "gui"); //#warninig unflexibility with m_renderPassName
																											   //}
		//gEnv->pRenderer->addOffscreenIndexedDraw(drawInfo[j], gEnv->pRenderer->getRenderPass(m_renderPassName), m_draw.offscreen->getFramebuffer());
	}
	/*uint32_t tmpI;
	for (size_t j = 0; j < groups.size(); j++) {
		drawInfo[j].bindDescriptorSets(gEnv->pRenderer->getShader(groups[j].shader)->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(descriptorSets[groups[j].ids[0]]));
		drawInfo[j].bindPipeline(gEnv->pRenderer->getPipeline(pipelineNames[groups[j].ids[0]]));
		drawInfo[j].bindVertexBuffers(buffers[groups[j].ids[0]], 1, m_draw.gOffsets[j].data());
		drawInfo[j].bindIndexBuffer(gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.buffer, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.offset, VK_INDEX_TYPE_UINT32);
		drawInfo[j].drawIndexed(groups[j].indicesCount, 1, groups[j].firstIndexPos, 0, 0);//#enchancement for better flexibility #done
																						  //drawInfo[groups[j].ids[0]];
																						  //gEnv->pRenderer->addIndexedDraw(drawInfo[groups[j].ids[0]], gEnv->pRenderer->getRenderPass(m_renderPassName),"gui"); //#warninig unflexibility with m_renderPassName
		gEnv->pRenderer->addIndexedDraw(drawInfo[j], gEnv->pRenderer->getRenderPass(m_renderPassName), "gui"); //#warninig unflexibility with m_renderPassName
																										   //}
	}*/
}

void GUI::loadWidgets()
{
	size_t tmpOffset;
	VirtualBuffer *tmpBuffer = nullptr;
	std::vector<std::string> shaderNames;
	std::vector<uint32_t> descriptorSets;
	std::vector<std::string> pipelineNames;
	std::vector<VkBuffer> buffers;
	uint32_t vertexc_widgetCount = 0;
	uint32_t vertexc_start;
	uint32_t vertexc_end;
	uint32_t vertext_widgetCount = 0;
	uint32_t vertext_start;
	uint32_t vertext_end;
	std::vector<uint32_t> tmpIndices;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indicesSizes;
	std::vector<uint32_t> indicesOffsets; //The offset of the highset value use by the "pack" of indices, [0,1,2,0,2,3] 3 is indexOffset of this list
	std::vector<uint32_t> indicesPos;

	std::vector<uint32_t> rawIndices;

	uint32_t tmpIndicesOffset = 0;
	uint32_t tmpIndicesPos = 0;
	bool descriptorColorSetCreated = false;
	bool descriptorTexSetCreated = false;

	

	for (size_t i = 0; i < m_widgets.size();i++) {
		if (m_widgets[i]->getClassName()=="Panel") {
			printf("PANEL\n");
			//Widget* tw = m_widgets.back(); //tmp widget
			//m_widgets[i]->move(offset2D(-1.0f,-1.0f));
			//m_widgets[i]->resize(extent2D(1.0f,1.0f));
			Widget* tw = m_widgets[i]; //tmp widget
			VertexC* tmpV = new VertexC[meshhelper::QUAD_VERTICES_COUNT];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT];
			tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			tw->gData(tmpV);
			tw->gIndices(tmpI);
			tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(tw->getBufferId()).bufferInfo.offset;
			m_draw.gOffset.push_back(tmpOffset);
			printf("%i ",(int)tmpOffset);
			printf("%i %i\n", (int)tw->gDataSize(), (int)(tw->gDataSize() + tmpOffset));
			//printf("%i %i\n", (int)tw->gIndicesSize(), (int)(tw->gDataSize() + tw->gIndicesSize() + tmpOffset));
			
			for (size_t w = 0; w < meshhelper::QUAD_VERTICES_COUNT;w++) {
				printf("%f %f %f\n", tmpV[w].pos.x, tmpV[w].pos.y, tmpV[w].pos.z);
			}
		

			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);
			//gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gIndicesSize(), tmpOffset + tw->gDataSize(), tmpI);


			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, tw->getDescriptorsIds()[0]);
			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[0]);

			//addIndices(tmpI);
			
			for (size_t k = 0; k < meshhelper::QUAD_INDICES_COUNT;k++) {
				tmpIndices.push_back(tmpI[k]/*+tmpIndicesOffset*/);
			}
			
			indicesPos.push_back(tmpIndicesPos);
			tmpIndicesPos += 6;
			indicesSizes.push_back(6);
			indicesOffsets.push_back(4);
			tmpIndicesOffset += 4;
			
			std::vector<VkWriteDescriptorSet> writeDescriptor;

			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, 1);
			VirtualBuffer* tmp_vb = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			VkDescriptorBufferInfo* tmp_descri = &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo;
			/*if (!descriptorColorSetCreated) {
				for (size_t i = 0; i < m_draw.descriptorSetId.size(); i++)
				{
					printf("GUI descriptorSetCreation");
					gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[i]);

					//writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[i]), m_draw.descriptorSetTypes[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo));
					writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[i]), m_draw.descriptorSetTypes[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));

				}
				gEnv->pRenderer->addWriteDescriptorSet(writeDescriptor);
				gEnv->pRenderer->updateDescriptorSets();
				descriptorColorSetCreated = true;
			}*/
		/*	for (size_t i = 0; i < tw->getDescriptorsIds().size();i++) {
				gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, tw->getDescriptorsIds()[i]);
				writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(tw->getDescriptorsIds()[i]), tw->getDescriptorsType()[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo));
			}*/

			

			

			//VkDeviceSize gOffset[1];
			//gOffset[0] = 

			
			vertexc_widgetCount += 1;
			
			descriptorSets.push_back(m_draw.descriptorSetId[0]);
			shaderNames.push_back("color");
			pipelineNames.push_back("gui_col");
			buffers.push_back(tmpBuffer->bufferInfo.buffer);
			//SIndexedDrawInfo drawInfo = {};

			////drawInfo.bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(tw->getDescriptorsIds()[0]));
			//drawInfo.bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[0]));
			//drawInfo.bindPipeline(gEnv->pRenderer->getPipeline("gui"));
			//m_draw.gOffset.push_back(static_cast<VkDeviceSize>(tmpOffset));
			//
			////drawInfo.bindVertexBuffers(tmpBuffer->bufferInfo.buffer ,1, &tmpOffset);
			//drawInfo.bindVertexBuffers(tmpBuffer->bufferInfo.buffer, 1, m_draw.gOffset.data());
			//drawInfo.bindIndexBuffer(tmpBuffer->bufferInfo.buffer, tmpOffset + tw->gDataSize(), VK_INDEX_TYPE_UINT32);
			//drawInfo.drawIndexed(6, 1, 0, 0, 0);

			////gEnv->pRenderer->addIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass("gui"));
			//gEnv->pRenderer->addIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass(m_renderPassName),"gui");
			//gEnv->pRenderer->addOffscreenIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass(m_renderPassName));

			//gEnv->pRenderer->addOffscreenIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass("gui"), gEnv->pRenderer->getOffscreen("font")->getFramebuffer());
				//drawInfo.bindPipeline(gEnv->);
			delete[] tmpV;
			delete[] tmpI;
			//m_draw.draws.push_back(drawInfo);
		}

		if (m_widgets[i]->getClassName()=="guilabel") {
			printf("GuiLabel\n");
			Widget* tw = m_widgets[i];
			size_t woTextSize = tw->getText().size() - helper::countCharacter(tw->getText(), ' ');//text size without blank
			VertexT* tmpV = new VertexT[meshhelper::QUAD_VERTICES_COUNT*woTextSize];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT*woTextSize];
			tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			tw->gData(tmpV);
			tw->gIndices(tmpI);
			tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(tw->getBufferId()).bufferInfo.offset;
			m_draw.gOffset.push_back(tmpOffset);

			for (size_t w = 0; w < meshhelper::QUAD_VERTICES_COUNT; w++) {
				printf("%f %f %f %f %f\n", tmpV[w].pos.x, tmpV[w].pos.y, tmpV[w].pos.z, tmpV[w].tc.x, tmpV[w].tc.y);
			}

			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);
			
			for (size_t k = 0; k < meshhelper::QUAD_INDICES_COUNT*woTextSize; k++) {
				tmpIndices.push_back(tmpI[k] /*+ tmpIndicesOffset*/);
				//indices.push_back(22);
				printf("%i %i\n",tmpIndices.size()-1,tmpIndices.back());
			}
			indicesPos.push_back(tmpIndicesPos);
			tmpIndicesPos += 6 * woTextSize;
			indicesSizes.push_back(6 * woTextSize);
			indicesOffsets.push_back(4 * woTextSize);
			tmpIndicesOffset += 4*woTextSize;
			
			//std::vector<VkWriteDescriptorSet> writeDescriptor;
			/*for (size_t z = 0; z < tw->getText().size()*meshhelper::QUAD_VERTICES_COUNT;z++) {
				printf("%f %f %f\n",tmpV[z].pos.x, tmpV[z].pos.y, tmpV[z].pos.z);
			}
			*/
			/*for (size_t z = 0; z < tw->getText().size()*meshhelper::QUAD_INDICES_COUNT; z++) {
				//printf("%f %f %f\n", tmpV[z].pos.x, tmpV[z].pos.y, tmpV[z].pos.z);
				printf("%i %i\n",z+tmpIndicesOffset-(4*tw->getText().size()), indices[z+tmpIndicesOffset-(4*tw->getText().size())]/*+tmpIndicesOffset);
			}*/
			vertexc_widgetCount += 1;

			descriptorSets.push_back(m_draw.descriptorSetId[1]);
			shaderNames.push_back("font");
			pipelineNames.push_back("gui_font");
			buffers.push_back(tmpBuffer->bufferInfo.buffer);
		
			delete[] tmpV;
			delete[] tmpI;
		}


		/*SIndexedDrawInfo drawInfo = {};
		drawInfo.bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout, 1, gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[0]));
		drawInfo.bindPipeline(gEnv->pRenderer->getPipeline("gui"));
		drawInfo.bindVertexBuffers();*/

	}


	

	loadDescriptorSets();

	std::vector<uint32_t> groupMask;
	struct groupId{
		std::string shader;
		std::string pipeline;
		uint32_t descriptorSetId;
		std::vector<uint32_t> ids;
		uint32_t indicesCount = 0; //Number of indices 
		uint32_t firstIndexPos = 0; //The position of the first index in the indices list (the one send to the GPU)
		uint32_t lastIndexOffset = 0; //The offset you need to add to the "raw" indices
		//VkBuffer buffer;
		bool equal(groupId g) {
			if (g.shader != shader) {
				return false;
			}
			if (g.pipeline != pipeline) {
				return false;
			}
			if (g.descriptorSetId!=descriptorSetId) {
				return false;
			}
			/*if (g.descriptorSetId.size()!=descriptorSetId.size()) {
				return false;
			}*/
			/*for (size_t i = 0; i < descriptorSetId.size();i++) {
				if (g.descriptorSetId[i]!=descriptorSetId[i]) {
					return false;
				}
			}*/
			return true;
		}
	};
	std::vector<groupId> groups;
	groupId tmpGroup;
	bool tmpDone;
	uint32_t tj;
	for (size_t i = 0; i < vertexc_widgetCount; i++) {
		tmpGroup.shader = shaderNames[i];
		tmpGroup.pipeline = pipelineNames[i];
		tmpGroup.descriptorSetId = { descriptorSets[i] };
		if (groups.size() == 0) {
			groups.push_back(tmpGroup);

		}
		tmpDone = false; 
		tj = 0;
		for (size_t j = 0; j < groups.size();j++) {
			tj = j;
			if (tmpGroup.equal(groups[j])) {
				groupMask.push_back(j);
				tmpDone = true;
				break;
			}
			/*else {
				groups.push_back(tmpGroup);
				groupMask.push_back(groups.size()-1);
				
			}*/
			//groups[j].ids.push_back(i);
		}	
		if (!tmpDone) {
			groups.push_back(tmpGroup);
			groupMask.push_back(groups.size() - 1);
			tj += 1;
		}
		groups[tj].ids.push_back(i);
		
	}
	std::vector<VkDeviceSize> tmpGOffset;
	
	uint32_t tmpIndexLastPos = 0;
	int tmpa = 0;
	int tmpb = 0;
	for (size_t j = 0; j < groups.size();j++) {
		groups[j].firstIndexPos = tmpIndexLastPos;
		tmpIndexLastPos = 0;
		for (size_t k = 0; k < groups[j].ids.size();k++) {
			groups[j].indicesCount += indicesSizes[groups[j].ids[k]];
			
			for (size_t l = 0; l < indicesSizes[groups[j].ids[k]];l++) {
				tmpa = l + tmpIndexLastPos + groups[j].firstIndexPos;
				tmpb = l + indicesPos[groups[j].ids[k]];
				//indices[l + tmpIndexLastPos+groups[j].firstIndexPos] += groups[j].lastIndexOffset;
				tmpIndices[tmpb] += groups[j].lastIndexOffset;
			}
			tmpIndexLastPos += indicesSizes[groups[j].ids[k]];
			groups[j].lastIndexOffset += indicesOffsets[groups[j].ids[k]];
		}
		tmpGOffset.resize(groups[j].ids.size());
		for (size_t k = 0; k < groups[j].ids.size();k++) {
			tmpGOffset[k] = m_draw.gOffset[groups[j].ids[k]];
		}
		m_draw.gOffsets.push_back(tmpGOffset);
	}
	printf("\n\n\n\nIndices-----\n\n\n\n");
	indices.resize(tmpIndices.size());
	uint32_t tmplastIId = 0;
	for (size_t j = 0; j < groups.size();j++) {
		
		for (size_t k = 0; k < groups[j].ids.size();k++) {
			for (size_t l = 0; l < indicesSizes[groups[j].ids[k]];l++) {
				indices[l + tmplastIId] = tmpIndices[l+indicesPos[groups[j].ids[k]]];
			}
			tmplastIId += indicesSizes[groups[j].ids[k]];
		}
		printf("%i %i\n", j, groups[j].indicesCount);
	}

	gEnv->pRenderer->bufferSubData(gEnv->bbid, m_draw.indicesSize, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.offset, indices.data());
	std::vector<SIndexedDrawInfo> drawInfo;
	drawInfo.resize(groups.size());

	uint32_t tmpI;
	for (size_t j = 0; j < groups.size();j++) {
		//for (size_t i = 0; i < groups[j].ids.size();i++) {
			//drawInfo[j].bindDescriptorSets(gEnv->pRenderer->getShader(shaderNames[i])->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(descriptorSets[i]));
			//drawInfo[j].bindPipeline(gEnv->pRenderer->getPipeline(pipelineNames[i]));
			//drawInfo[j].bindDescriptorSets(gEnv->pRenderer->getShader(shaderNames[groups[j].ids[0]])->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(descriptorSets[groups[j].ids[0]]));
		//	gEnv->pRenderer->getShader(groups[j].shader)->getPipelineLayout();
		//	gEnv->pRenderer->getDescriptorSet(descriptorSets[groups[j].ids[0]]);
			drawInfo[j].bindDescriptorSets(gEnv->pRenderer->getShader(groups[j].shader)->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(descriptorSets[groups[j].ids[0]]));
			drawInfo[j].bindPipeline(gEnv->pRenderer->getPipeline(pipelineNames[groups[j].ids[0]]));
			//drawInfo[j].bindVertexBuffers(buffers[groups[j].ids[0]], m_draw.gOffset.size(), m_draw.gOffset.data());
			drawInfo[j].bindVertexBuffers(buffers[groups[j].ids[0]], m_draw.gOffsets[j].size(), m_draw.gOffsets[j].data());
			drawInfo[j].bindIndexBuffer(gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.buffer, gEnv->pMemoryManager->getVirtualBufferPtr(m_draw.indicesOffsetId)->bufferInfo.offset, VK_INDEX_TYPE_UINT32);
			//drawInfo[j].drawIndexed(indices.size(), 1, 0, 0, 0);//#enchancement for better flexibility
			//drawInfo[j].drawIndexed(groups[j].indicesCount, 1, 0, 0, 0);//#enchancement for better flexibility #done
			drawInfo[j].drawIndexed(groups[j].indicesCount, 1, groups[j].firstIndexPos, 0, 0);//#enchancement for better flexibility #done
			//drawInfo[groups[j].ids[0]];
			//gEnv->pRenderer->addIndexedDraw(drawInfo[groups[j].ids[0]], gEnv->pRenderer->getRenderPass(m_renderPassName),"gui"); //#warninig unflexibility with m_renderPassName
			gEnv->pRenderer->addIndexedDraw(drawInfo[j], gEnv->pRenderer->getRenderPass(m_renderPassName), "gui"); //#warninig unflexibility with m_renderPassName
		//}
	}

}

void GUI::reorderWidgets()
{
	std::vector<Widget*> copy = m_widgets;
	m_widgets.resize(0);

	//Criteria
	std::vector<std::string> clrShaderedWidgets = { "Panel" }; //TexturedWidgets = !clrShaderedWidgets

	//Process clrShaderedWidgets
	for (size_t i = 0; i < copy.size(); i++) {
		if (helper::contains(copy[i]->getClassName(), clrShaderedWidgets)) {
			m_widgets.push_back(copy[i]);
		}
	}
	//Process texShaderedWidgets
	for (size_t i = 0; i < copy.size(); i++) {
		if (!helper::contains(copy[i]->getClassName(), clrShaderedWidgets)) {
			m_widgets.push_back(copy[i]);
		}
	}


}

void GUI::loadDescriptorSets()
{
	/*gli::texture2D tex(gli::load(gEnv->getAssetpath()+"textures/Fanatic_TriWave_1024.ktx"));
	assert(!tex.empty());

	VkImageCreateInfo imageCreateInfo = vkTools::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_BC2_UNORM_BLOCK;
	imageCreateInfo.mipLevels = tex.levels();
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	imageCreateInfo.extent = { (uint32_t)tex.dimensions().x, (uint32_t)tex.dimensions().y, 1};
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	//*uint8_t* data;
	//memcpy(data, tex.data(), tex.size());

	gEnv->pRenderer->createTexture(&test_texId, imageCreateInfo, (uint8_t*)tex.data(),1024,1024);*/
	//gEnv->pRenderer->loadTextureFromFile(&test_texId, gEnv->getAssetpath() + "textures/Fanatic_TriWave_1024.ktx", VK_FORMAT_BC2_UNORM_BLOCK);
	
	
	
	/*gEnv->pRenderer->loadTextureFromFile(&test_texId, gEnv->getAssetpath() + "textures/segoeui40.ktx", VK_FORMAT_BC2_UNORM_BLOCK);
	test_imgDescriptor = 
		vkTools::initializers::descriptorImageInfo(gEnv->pRenderer->getTexture(test_texId)->sampler, 
			gEnv->pRenderer->getTexture(test_texId)->view,
			gEnv->pRenderer->getTexture(test_texId)->imageLayout);*/
	
	CFont* ft = gEnv->pRessourcesManager->getCFont(m_fontName, m_fontSize);


	printf("GUI descriptorSetCreation");

	std::vector<VkWriteDescriptorSet> writeDescriptor;

	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[0]);
	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[0]), 
		m_draw.descriptorSetTypes[0], 
		0, 
		&gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));

	
	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("font")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[1]);
	
	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[1]),
		m_draw.descriptorSetTypes[0],
		0,
		&gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));
		

	/*writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[1]),
		m_draw.descriptorSetTypes[1],
		1,
		&test_imgDescriptor));*/

	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[1]),
		m_draw.descriptorSetTypes[1],
		1,
		ft->getDescriptorImageInfo()));

	/*writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(ft->getDescriptorSetId()),
		m_draw.descriptorSetTypes[1],
		1,
		ft->getDescriptorImageInfo()));*/
		//&gEnv->pRessourcesManager->getCFont("segoeui",12)->getDescriptorImageInfo()));
		
	gEnv->pRenderer->addWriteDescriptorSet(writeDescriptor);
	gEnv->pRenderer->updateDescriptorSets();
	

}

XMLWidget GUI::getWidgetXMLInfo(mxml_node_t * node)
{
	XMLWidget xw = {};
	XMLParser parser(m_file);
	size_t attribNameId = -1, attribWidthId = -1, attribHeightId = -1, attribPosXId = -1, attribPosYId = -1;

	for (size_t i = 0; i < parser.getXData(node).attributeName.size(); i++) {
		if (parser.getXData(node).attributeName[i] == "name" && attribNameId==-1) {
			attribNameId = i;
			xw.name = parser.getXData(node).attributeValue[attribNameId];
		}
		else if (parser.getXData(node).attributeName[i] == "width" && attribWidthId == -1) {
			attribWidthId = i;
			xw.width = std::stof(parser.getXData(node).attributeValue[attribWidthId]);
		}
		else if (parser.getXData(node).attributeName[i] == "height" && attribHeightId == -1) {
			attribHeightId = i;
			xw.height = std::stof(parser.getXData(node).attributeValue[attribHeightId]);
		}
		else if (parser.getXData(node).attributeName[i] == "posx" && attribPosXId == -1) {
			attribPosXId = i;
			xw.x = std::stof(parser.getXData(node).attributeValue[attribPosXId]);
		}
		else if (parser.getXData(node).attributeName[i] == "posy" && attribPosYId == -1) {
			attribPosYId = i;
			xw.y = std::stof(parser.getXData(node).attributeValue[attribPosYId]);
		}
	}

	return xw;
}

void GUI::addDoubleSetting(std::string name, double value)
{
	m_settings.doubleSettingsName.push_back(name);
	m_settings.doubleSettings.push_back(value);
}
/*
void GUI::addCreator(std::string name, std::function<void(mxml_node_t* t)> creator) //void (GUI::*creator)(mxml_node_t* t))//std::function<void(mxml_node_t* t)> creator)
{
	m_creators.elementNames.push_back(name);
	m_creators.creators.push_back(creator);
}*/

void GUI::addCreator(std::string name, void(GUI::*&& _Func)(mxml_node_t *t))// , void (GUI::*&&loader)(mxml_node_t *t))
{
	m_elementNames.push_back(name);
	m_elementsCount.push_back(0);
	m_creators.push_back(std::bind(_Func, this, std::placeholders::_1));
}


void GUI::searchAndExecute(mxml_node_t * node)
{
	XMLParser parser(m_file);
	for (size_t i = 0; i < m_creators.size();i++) {
		if (parser.getXData(node).elementName==m_elementNames[i]) {
			m_creators[i](node);
		}
	}
}

void GUI::creator_Panel(mxml_node_t * t)
{
	printf("Panel creator : %s\n", m_file.c_str());
	printf("%f %f\n",getNextPosition().x, getNextPosition().y);

	size_t classId = helper::find("Panel", m_elementNames);
	XMLWidget xw = getWidgetXMLInfo(t);
	addWidget(new Panel("panel", rect2D(getNextPosition(), extent2D(guitools::getTextSize(xw.name, "./data/fonts/segoeui.ttf", m_fontSize).width, 20)), glm::uvec4(255,255,255,255), glm::uint(255), false, false));
	m_elementsCount[classId] += 1;

	m_draw.vertClrSize += m_widgets.back()->gDataSize();
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

	m_widgets.back()->setName("Panel"+ std::to_string(m_elementsCount[classId]));
	m_widgets.back()->setDepth(0.24);

	printf("%f %f\n", getNextPosition().x, getNextPosition().y);
}

void GUI::creator_guilabel(mxml_node_t * t)
{
	size_t classId = helper::find("Label", m_elementNames);
	XMLWidget xw = getWidgetXMLInfo(t);

	addWidget(new guilabel(xw.name, getNextPosition(), "segoeui", m_fontSize, false, false));

	m_elementsCount[classId] += 1;

	m_draw.vertTexSize += m_widgets.back()->gDataSize();
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

	m_widgets.back()->setName("guilabel" + std::to_string(m_elementsCount[classId]));
	m_widgets.back()->setDepth(0.24);

	

}

void GUI::creator_menu(mxml_node_t * t)
{
	size_t classId = helper::find("Menu", m_elementNames); 

	float offx = 10.0f;
	float offy = 5.0f;
	

	XMLWidget xw = getWidgetXMLInfo(t);
	extent2D dim = guitools::getTextSize(xw.name, "./data/fonts/segoeui.ttf", m_fontSize);
	rect2D bound = rect2D(getNextPosition(), dim);
	rect2D panelBound = bound;
	panelBound.extent.width += offx*2;
	panelBound.extent.height += offy*2;
	offset2D labelPos = bound.offset;
	labelPos.x += offx;
	labelPos.y += offy;
	addWidget(new Panel("menu", panelBound,glm::uvec4(255,255,255,255),glm::uint(255), false, false));
	m_draw.vertClrSize += m_widgets.back()->gDataSize();
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();
	addWidget(new guilabel(xw.name, rect2D(labelPos, bound.extent), m_fontName, m_fontSize, false, false));
	m_draw.vertClrSize += m_widgets.back()->gDataSize();
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

	m_elementsCount[classId] += 1;

	m_draw.vertClrSize += m_widgets.back()->gDataSize();
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

	m_widgets.back()->setName("menu" + std::to_string(m_elementsCount[classId]));
	m_widgets.back()->setDepth(0.24);

}
