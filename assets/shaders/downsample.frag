#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 o_color;

layout (set = 0, binding = 0) uniform sampler2D image;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy) * 2;
	o_color = vec4(texelFetch(image, coord, 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(1, 0), 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(0, 1), 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(1, 1), 0).rgb * 0.25, 1.0);
}
