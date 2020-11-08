#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define ELEMENT_SET 0
#include "element.dsl"

layout (location = 0) in vec4 i_color;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in flat uint i_id;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = i_color * texture(images[i_id], i_uv);
}
