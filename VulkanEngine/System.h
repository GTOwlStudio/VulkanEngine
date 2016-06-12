#pragma once

#include "ISystem.h"

class CSystem : public ISystem
{
public:
	CSystem();
	~CSystem();

	IInput* GetIInput() override { return m_env.pInput; };
	TEngineBase* GetTEngineBase() override { return m_env.pTEngine; };


private:
	bool InitInput(const SSystemInitParams& startupParams);

	bool InitEngine(const SSystemInitParams& initParams);
	

private:
	SSystemGlobalEnvironment &m_env;



};