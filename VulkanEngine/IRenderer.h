#pragma once

#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;

#include <Windows.h>

///**/
//
//struct SWindGrid 
//{
//	int m_nWidth;
//	int m_nHeight;
//	float m_fCellSize;
//	
//	vec3 m_vCentr;
//
//	SWindGrid() {};
//};
//
//class CRenderCamera
//{
//public:
//	CRenderCamera();
//	CRenderCamera(const CRenderCamera& Cam);
//	void Copy(const CRenderCamera& Cam);
//
//	void LookAt(const vec3& Eye, const vec3& ViewRefpt, const vec3& ViewUp);
//	void Perspective(float YFov, float Aspect, float NDist, float FDist);
//	vec3 ViewDir() const;
//	vec3 ViewDirOffAxis() const;
//
//	void Translate(const vec3& trans);
//	void Rotate(const mat3);
//
//	vec3 vX, vY, vZ;
//	vec3 vOrigin;
//	float fWL, fWR, fWB, fWT; //W for viewport and L,R,B,T for left, roght, bottom, top
//	float fNear, fFar;
//
//};
//
//CRenderCamera::CRenderCamera()
//{
//	vX = vec3(1, 0, 0);
//	vY = vec3(0, 1, 0);
//	vZ = vec3(0, 0, 1);
//}
//
//CRenderCamera::CRenderCamera(const CRenderCamera& Cam)
//{
//	Copy(Cam);
//}
//
//void CRenderCamera::Copy(const CRenderCamera& Cam) {
//	vX = Cam.vX;
//	vY = Cam.vY;
//	vZ = Cam.vZ;
//	vOrigin = Cam.vOrigin;
//	fNear = Cam.fNear;
//	fFar = Cam.fFar;
//	fWL = Cam.fWL;
//	fWR = Cam.fWR;
//	fWB = Cam.fWB;
//	fWT = Cam.fWT;
//}
//
//void CRenderCamera::LookAt(const vec3& Eye, const vec3& ViewRefpt, const vec3& ViewUp) {
//	
//}

class CSystem;

struct IRenderer
{
	virtual ~IRenderer() {}
	virtual void Init() = 0;
	virtual void InitVulkan() = 0;
	virtual void render() = 0;
	virtual void addGraphicPipeline(VkGraphicsPipelineCreateInfo pipelineCreateInfo, VkPipelineVertexInputStateCreateInfo inputState, std::string name) = 0;

};