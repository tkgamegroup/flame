#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;
layout (location = 3) in vec3 i_tangent;

layout (location = 4) in vec4 i_mvp_col0;
layout (location = 5) in vec4 i_mvp_col1;
layout (location = 6) in vec4 i_mvp_col2;
layout (location = 7) in vec4 i_mvp_col3;
layout (location = 8) in vec4 i_nor_col0;
layout (location = 9) in vec4 i_nor_col1;
layout (location = 10) in vec4 i_nor_col2;
layout (location = 11) in vec4 i_nor_col3;

layout (location = 0) out vec3 o_normal;

void main()
{
	o_normal = vec3(mat4(i_nor_col0, i_nor_col1, i_nor_col2, i_nor_col3) * vec4(i_normal, 0));
	gl_Position = mat4(i_mvp_col0, i_mvp_col1, i_mvp_col2, i_mvp_col3) * vec4(i_pos, 1.0);
}
