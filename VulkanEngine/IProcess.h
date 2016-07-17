#pragma once
struct IProcess 
{
	virtual ~IProcess();
	virtual bool Init() = 0;
	virtual void Update() = 0;
	virtual void ShutDown() = 0;
};