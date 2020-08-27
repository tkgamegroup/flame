#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

layout (set = 0, binding = 0) uniform sampler2D image;

layout (push_constant) uniform PushConstantT
{
	vec2 pxsz;
}pc;

void main()
{
	o_color = vec4(texture(image, i_uv + vec2(-pc.pxsz.x, -pc.pxsz.y)).rgb * 0.25 +
		texture(image, i_uv + vec2(+pc.pxsz.x, -pc.pxsz.y)).rgb * 0.25 +
		texture(image, i_uv + vec2(+pc.pxsz.x, +pc.pxsz.y)).rgb * 0.25 +
		texture(image, i_uv + vec2(-pc.pxsz.x, +pc.pxsz.y)).rgb * 0.25, 1.0);
}
