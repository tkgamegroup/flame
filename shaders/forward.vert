#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec4 i_col0;
layout (location = 2) in vec4 i_col1;
layout (location = 3) in vec4 i_col2;
layout (location = 4) in vec4 i_col3;

void main()
{
	gl_Position = mat4(i_col0, i_col1, i_col2, i_col3) * vec4(i_pos, 1.0);
}
