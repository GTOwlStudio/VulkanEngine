#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;

layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	vec4 color = texture(samplerColor, inUV);
	outFragColor = vec4(0.10*vec3(color.r), color.r);
	//outFragColor = vec4(color.r, 0.0, 0.0, color.r);
	//float alpha = color.r;
	//outFragColor = 0.8*vec4(color.r);
	/*if (color.r==0.0f){
		//alpha = 1.0;
		outFragColor = vec4(0.8*vec3(color.r), 1.0f);
	}*/
	
	//outFragColor = vec4(0.8, vec3(color.r), alpha);
	//outFragColor = vec4(vec3(0.1), 1.0);
	//outFragColor = vec4(1.0, 1.0, 1.0, color) * vec4(1.0,0.0,1.0,1.0);
	//outFragColor = vec4(1.0f, 1.0f, 1.0f, 1.0);
}