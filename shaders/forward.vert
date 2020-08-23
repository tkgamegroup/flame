#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;

layout (push_constant) uniform PushConstantT
{
	mat4 matrix;
}pc;

void main()
{
	gl_Position = pc.matrix * vec4(i_pos, 1.0);
}
