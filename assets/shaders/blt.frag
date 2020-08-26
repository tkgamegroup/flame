#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 i_coord;

layout (location = 0) out vec4 o_color;

layout (set = 0, binding = 0) uniform sampler2D image;

layout (push_constant) uniform PushConstantT
{
	vec4 range;
}pc;

void main()
{
	o_color = texelFetch(image, ivec2(pc.range.xy + (pc.range.zw - pc.range.xy) * i_coord), 0);
}
