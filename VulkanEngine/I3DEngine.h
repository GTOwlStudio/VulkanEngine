#pragma once

//#include "IRenderer.h"
#include "IProcess.h"

struct ISystem;

struct I3DEngine : public IProcess
{
	virtual bool Init() = 0;

	virtual void Update() = 0;
	virtual void Release() = 0;
	virtual void ShutDown() = 0;

	/*virtual IRenderNode* CreateRenderNode(EERType type) = 0;
	virtual void DeleteRenderNode(IRenderNode* pRenderNode) = 0;*/
};

struct SRenderingPassInfo
{
	//static SRenderingPassInfo CreateGeneralPassRenderingInfo();
};