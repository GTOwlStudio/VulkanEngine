#pragma once

struct ISystem;
struct TEngineBase;
struct IInput;

struct SSystemInitParams
{
	void* hInstance;
	void* hWnd;

	ISystem* pSystem;

	SSystemInitParams()
	{
		hInstance = NULL;
		hWnd = NULL;

		pSystem = NULL;

	}

};

struct SSystemGlobalEnvironment
{

	TEngineBase* pTEngine;
	IInput* pInput;

};

struct ISystem
{

	virtual ~ISystem() {}

	virtual void Render() = 0;

	virtual TEngineBase* GetTEngineBase() = 0;
	virtual IInput* GetIInput() = 0;


};