#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#ifndef NO_COORD
layout (location = 0) out vec2 o_coord;
#endif

void main()
{
	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 2.0),
		vec2(2.0, 0.0)
	};
	vec2 coord = vs[gl_VertexIndex];
#ifndef NO_COORD
	o_coord = coord;
#endif
	gl_Position = vec4(coord * 2.0 - 1.0, 1.0, 1.0);
}
