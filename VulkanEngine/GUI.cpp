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

	//addWidget(new Label("Sample", offset2D(50,50)));

	//addWidget(new Panel("Panel", rect2D(100, 100, 100, 100)));-
	//float matprojpar[4] = {0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight()};
	m_draw.UBO.projection = glm::ortho(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());
	//m_draw.UBO.projection = mat4(1.0f);
	//m_draw.UBO.projection = glm::ortho<float>(0.0f, (float)gEnv->pSystem->getWidth(), 0.0f, (float)gEnv->pSystem->getHeight());//GUI coherent 
	m_draw.UBO_bufferId = gEnv->pMemoryManager->requestMemory(sizeof(m_draw.UBO), "gUBO", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	printf("GUI requestedDescriptorSet\n");
	m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, "color"));
	//m_draw.descriptorSetId.push_back(gEnv->pRenderer->requestDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, "texture"));
	m_draw.descriptorSetTypes.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
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
	
	gEnv->pRenderer->addGraphicsPipeline(gEnv->pRenderer->getShader("color"), gEnv->pRenderer->getRenderPass(m_renderPassName), "gui", true);
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
		np.y = m_widgets[i]->getBoundary().offset.y + m_widgets[i]->getBoundary().extent.height;
		if (static_cast<uint32_t>(np.x)>=gEnv->pSystem->getWidth()) {
			np.x = 0;
		}
	}
	return np;
}

void GUI::loadCreator()
{
	//addCreator("Panel", &GUI::creator_Panel);
	addCreator("Panel", &GUI::creator_Panel);
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
			printf("%i ",(int)tmpOffset);
			printf("%i %i\n", (int)tw->gDataSize(), (int)(tw->gDataSize() + tmpOffset));
			printf("%i %i\n", (int)tw->gIndicesSize(), (int)(tw->gDataSize() + tw->gIndicesSize() + tmpOffset));
			
			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gDataSize(), tmpOffset, tmpV);
			gEnv->pRenderer->bufferSubData(gEnv->bbid, tw->gIndicesSize(), tmpOffset + tw->gDataSize(), tmpI);


			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, tw->getDescriptorsIds()[0]);
			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[0]);

			std::vector<VkWriteDescriptorSet> writeDescriptor;

			//gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, 1);
			VirtualBuffer* tmp_vb = gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId());
			VkDescriptorBufferInfo* tmp_descri = &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo;
			for (size_t i = 0; i < m_draw.descriptorSetId.size();i++) 
			{
				printf("GUI descriptorSetCreation");
				gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, m_draw.descriptorSetId[i]);

				//writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[i]), m_draw.descriptorSetTypes[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo));
				writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[i]), m_draw.descriptorSetTypes[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(gEnv->pMemoryManager->getUniformBufferId(m_draw.UBO_bufferId))->bufferInfo));

			}
		/*	for (size_t i = 0; i < tw->getDescriptorsIds().size();i++) {
				gEnv->pRenderer->createDescriptorSet(gEnv->pRenderer->getDescriptorPool(0), gEnv->pRenderer->getShader("color")->getDescriptorSetLayoutPtr(), 1, tw->getDescriptorsIds()[i]);
				writeDescriptor.push_back(vkTools::initializers::writeDescriptorSet(*gEnv->pRenderer->getDescriptorSet(tw->getDescriptorsIds()[i]), tw->getDescriptorsType()[i], 0, &gEnv->pMemoryManager->getVirtualBufferPtr(tw->getBufferId())->bufferInfo));
			}*/

			

			gEnv->pRenderer->addWriteDescriptorSet(writeDescriptor);
			gEnv->pRenderer->updateDescriptorSets();

			//VkDeviceSize gOffset[1];
			//gOffset[0] = 


			SIndexedDrawInfo drawInfo = {};

			//drawInfo.bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(tw->getDescriptorsIds()[0]));
			drawInfo.bindDescriptorSets(gEnv->pRenderer->getShader("color")->getPipelineLayout(), 1, gEnv->pRenderer->getDescriptorSet(m_draw.descriptorSetId[0]));
			drawInfo.bindPipeline(gEnv->pRenderer->getPipeline("gui"));
			m_draw.gOffset[0] = {static_cast<VkDeviceSize>(tmpOffset)};
			//drawInfo.bindVertexBuffers(tmpBuffer->bufferInfo.buffer ,1, &tmpOffset);
			drawInfo.bindVertexBuffers(tmpBuffer->bufferInfo.buffer, 1, m_draw.gOffset);
			drawInfo.bindIndexBuffer(tmpBuffer->bufferInfo.buffer, tmpOffset + tw->gDataSize(), VK_INDEX_TYPE_UINT32);
			drawInfo.drawIndexed(6, 1, 0, 0, 0);

			//gEnv->pRenderer->addIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass("gui"));
			gEnv->pRenderer->addIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass(m_renderPassName));
			//gEnv->pRenderer->addOffscreenIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass(m_renderPassName));

			//gEnv->pRenderer->addOffscreenIndexedDraw(drawInfo, gEnv->pRenderer->getRenderPass("gui"), gEnv->pRenderer->getOffscreen("font")->getFramebuffer());
				//drawInfo.bindPipeline(gEnv->);

			//m_draw.draws.push_back(drawInfo);
		}
	}
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
	addWidget(new Panel("panel", rect2D(getNextPosition(), extent2D(100, 20))));


	printf("%f %f\n", getNextPosition().x, getNextPosition().y);
}
