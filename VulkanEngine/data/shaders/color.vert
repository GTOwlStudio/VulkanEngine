#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (binding = 0) uniform UBO
{
	mat4 mat;
} ubo;

layout (location = 0) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void){

	gl_Position = ubo.mat * vec4(inPos.xy, -inPos.z,1.0);
	
	outColor = inColor;
}

