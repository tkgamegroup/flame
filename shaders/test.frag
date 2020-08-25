#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 i_coord;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(0.8 * i_coord.x, 0.9 * i_coord.y, 0.2, 1.0);
}
