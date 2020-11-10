#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#include "post.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	vec3 color = texture(image, i_uv).rgb;
	float brightness = max(color.r, max(color.g, color.b));
	float contribution = max(0, brightness - 1.0);
	contribution /= max(brightness, 0.00001);
	o_color = vec4(color * contribution, 1.0);
}
