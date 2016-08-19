#pragma once

void buildCommandBuffer()
{
	
	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearColor clearValue[2];
	clearValue[0].color = { 0.25f, 0.25f, 0.25f, 1.0f };
	clearValue[1].depthStencil = {1.0f, 0};

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = gEnv->pSystem->getWidth();
	renderPassBeginInfo.renderArea.extent.height = gEnv->pSystem->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	for (int32_t i = 0;i<m_drawCmdBuffers.size()) 
	{
		renderPassBeginInfo.framebuffer = m_frameBUffers[i];

		VK_CEHCK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENT_INLINE);

		VkViewport viewport = vkTools::initializers::viewport(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0.0f, 1.0f);
		vkCmdSetViewport(m_drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(gEnv->pSystem->getWidth(), gEnv->pSystem->getHeight(), 0, 0);
		vkCmdSetScissor(m_drawCmdBuffers[i], 0, 1, &scissor);



		vkCmdEndRenderPass(m_drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));

	}

}
