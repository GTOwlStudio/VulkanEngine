#pragma once

#include <glm\glm.hpp>
#include <vulkan\vulkan.h>



struct GData
{
	float pos[3];
	float col[3];
};

namespace guiTools
{

	bool intersection(VkOffset2D a, VkRect2D b);
	bool intersection(glm::vec2 a, VkRect2D b);

}