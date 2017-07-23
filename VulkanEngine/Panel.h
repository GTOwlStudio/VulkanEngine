#pragma once
#include "System.h" //F**CKING IMPORTANT, This header must stay on top of the other

#include "Widget.h"
#include "MeshHelper.h"

class Panel :
	public Widget
{
public:
	Panel(std::string name, rect2D boundary, glm::uvec4 color = glm::uvec4(255, 255, 255, 255), glm::uint refValue = 255, bool genIdx = true, bool genVtx = true);
	virtual ~Panel();
	
	virtual VkDeviceSize gSize();
	virtual VkDeviceSize gDataSize();
	virtual VkDeviceSize gIndicesSize();
	virtual void gData(void* arr);
	virtual void gIndices(void* arr);

	glm::vec4 getColor() const;
	

protected:
	glm::uvec4 m_color;
	glm::uint m_colorRef = 255; //Usely it's 255, you do the value/refValue(0/255)
};

