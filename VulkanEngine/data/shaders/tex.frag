#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;

layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	vec4 color = texture(samplerColor, inUV);
	outFragColor = vec4(color.rgb, 1.0);
	//outFragColor = vec4(1.0, 1.0, 1.0, color) * vec4(1.0,0.0,1.0,1.0);
	//outFragColor = vec4(1.0f, 1.0f, 1.0f, 1.0);
}