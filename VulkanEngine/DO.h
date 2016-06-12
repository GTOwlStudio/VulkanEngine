#pragma once
/*
#include <iostream>
#include <vector>*/


#include "TCore.h"

#include <vulkan\vulkan.h>
//#include "guiButton.h"
//#include "guiTools.h"
//#include "vulkantools.h"
//#include "Input.h"


class TCore;

class DO {
public:
	DO(TCore &core);
	~DO();

protected:
	TCore &_core;


};