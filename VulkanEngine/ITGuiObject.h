#pragma once

#include <vector>
#include "guiTools.h"



class ITGuiObject
{
public:
	virtual ~ITGuiObject() {}

	virtual void updateLogic() = 0;

	virtual const std::vector<GData> &getData() = 0;
	virtual const std::vector<uint32_t> &getIndices() = 0;


};
