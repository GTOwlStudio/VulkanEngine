#pragma once

#include <vector>
#include <assert.h>

#include "Framebuffer.h"

class CTester
{
public:
	CTester();
	~CTester();
	void load();
	void updateUniformBuffer();
	
protected:
	void loadGraphics();
	struct {
		struct {
			glm::mat4 projection;
		} UBO;
		size_t UBO_bufferId;
		uint32_t descriptorSetId;
		uint32_t bufferId;
		VkDeviceSize gOffsets[2];
		std::vector<SIndexedDrawInfo> draws;
	} m_draw;

};

