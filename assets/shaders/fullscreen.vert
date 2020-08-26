#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 o_uv;

void main()
{
	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 2.0),
		vec2(2.0, 0.0)
	};
	vec2 coord = vs[gl_VertexIndex];
	o_uv = coord;
	gl_Position = vec4(coord * 2.0 - 1.0, 1.0, 1.0);
}
