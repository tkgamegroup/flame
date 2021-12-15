#include "plain3d.pll"

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec4 i_col;

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = i_col;
	gl_Position = pc.mvp * vec4(i_pos, 1);
}
