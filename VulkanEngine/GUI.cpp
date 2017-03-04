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

	m_draw.indicesOffsetId = gEnv->pMemoryManager->requestMemory(m_draw.indicesSize, "INDICES");

	//addWidget(new Label("Sample", offset2D(50,50)));

	//addWidget(new Panel("Panel", rect2D(100, 100, 100, 100)));-
	//float matprojpar[4] = {0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight()};
	m_draw.UBO.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());
	//m_draw.UBO.projection = mat4(1.0f);
	//m_draw.UBO.projection = glm::ortho<float>(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());//GUI coherent 
	m_draw.UBO_bufferId = gEnv->pMemoryManager->requestMemory(sizeof(m_draw.UBO), "gUBO", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	printf("GUI requestedDescriptorSet\n");
	m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, "color"));
	m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(std::vector<VkDescriptorType>{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, "tex"));
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
	
	////m_draw.offscreen = gEnv->pRenderer->addOffscreen("gui");
	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass("gui"), "gui");
	//gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("tex"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_tex", true);
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_col", true);
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("tex"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui_tex", true);
	printf("GUI renderPass and Graphics Pipeline Created");
	printf("%i object to be load\n", static_cast<int>(m_widgets.size()));
	loadWidgets();
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
	std::vector<uint32_t> indices;
	std::vector<uint32_t> indicesSizes;
	std::vector<uint32_t> indicesOffsets; //The offset of the highset value use by the "pack" of indices, [0,1,2,0,2,3] 3 is indexOffset of this list
	
	uint32_t tmpIndicesOffset = 0;
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
				indices.push_back(tmpI[k]/*+tmpIndicesOffset*/);
			}
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
			VertexT* tmpV = new VertexT[meshhelper::QUAD_VERTICES_COUNT*tw->getText().size()];
			uint32_t* tmpI = new uint32_t[meshhelper::QUAD_INDICES_COUNT*tw->getText().size()];
			tmpBuffer = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			tw->gData(tmpV);
			tw->gIndices(tmpI);
			tmpOffset = gEnv->pMemoryManager->getVirtualBuffer(tw->getBufferId()).bufferInfo.offset;
			m_draw.gOffset.push_back(tmpOffset);

			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);
			for (size_t k = 0; k < meshhelper::QUAD_INDICES_COUNT*tw->getText().size(); k++) {
				indices.push_back(tmpI[k] /*+ tmpIndicesOffset*/);
				//indices.push_back(22);
				printf("%i %i\n",indices.size()-1,indices.back());
			}
			indicesSizes.push_back(6 * tw->getText().size());
			indicesOffsets.push_back(4 * tw->getText().size());
			tmpIndicesOffset += 4*tw->getText().size();
			
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
			shaderNames.push_back("tex");
			pipelineNames.push_back("gui_tex");
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
	for (size_t j = 0; j < groups.size();j++) {
		groups[j].firstIndexPos = tmpIndexLastPos;
		tmpIndexLastPos = 0;
		for (size_t k = 0; k < groups[j].ids.size();k++) {
			groups[j].indicesCount += indicesSizes[groups[j].ids[k]];
			
			for (size_t l = 0; l < indicesSizes[groups[j].ids[k]];l++) {
				tmpa = l + tmpIndexLastPos + groups[j].firstIndexPos;
				indices[l + tmpIndexLastPos+groups[j].firstIndexPos] += groups[j].lastIndexOffset;
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

void GUI::loadDescriptorSets()
{
	printf("GUI descriptorSetCreation");

	std::vector<VkWriteDescriptorSet> writeDescriptor;

	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[0]);

	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[0]), 
		m_draw.descriptorSetTypes[0], 
		0, 
		&gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));

	
	gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("tex")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[1]);
	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[1]),
		m_draw.descriptorSetTypes[0],
		0,
		&gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));
		

	writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[1]),
		m_draw.descriptorSetTypes[1],
		1,
		&gEnv->pRessourcesManager->getCFont("segoeui",40)->getDescriptorImageInfo()));
		
	gEnv->pRenderer->addWriteDescriptorSet(writeDescriptor);
	gEnv->pRenderer->updateDescriptorSets();
	

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
	m_creators.elementNames.push_back(name);
	m_creators.creators.push_back(std::bind(_Func, this, std::placeholders::_1));
}


void GUI::searchAndExecute(mxml_node_t * node)
{
	XMLParser parser(m_file);
	for (size_t i = 0; i < m_creators.creators.size();i++) {
		if (parser.getXData(node).elementName==m_creators.elementNames[i]) {
			m_creators.creators[i](node);
		}
	}
}

void GUI::creator_Panel(mxml_node_t * t)
{
	printf("Panel creator : %s\n", m_file.c_str());
	printf("%f %f\n",getNextPosition().x, getNextPosition().y);
	addWidget(new Panel("panel", rect2D(getNextPosition(), extent2D(100, 20)), glm::uvec4(255,255,255,255), glm::uint(255), true));
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

	printf("%f %f\n", getNextPosition().x, getNextPosition().y);
}

void GUI::creator_guilabel(mxml_node_t * t)
{
	XMLParser parser(m_file);
	printf("guilabel creator\n");
	size_t attribNameId = 0;
	for (size_t i = 0; i < parser.getXData(t).attributeName.size();i++) {
		if (parser.getXData(t).attributeName[i]=="name") {
			attribNameId = i;
			break;
		}
	}
	printf("%s\n", parser.getXData(t).attributeValue[attribNameId].c_str());
	addWidget(new guilabel(parser.getXData(t).attributeValue[attribNameId], getNextPosition(), "segoeui", 40, true));
	m_widgets.back()->setDepth(0.24);
	m_draw.indicesSize += m_widgets.back()->gIndicesSize();

}
