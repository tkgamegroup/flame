#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_normal;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(vec3(dot(i_normal, vec3(0, 0, 1))), 1.0);
}