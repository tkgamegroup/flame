#include "plain.pll"

layout (location = 0) in vec2 i_pos;

#ifdef USE_VERTEX_COLOR
layout (location = 1) in vec4 i_col;
layout (location = 0) out vec4 o_col;
#endif

void main()
{
#ifdef USE_VERTEX_COLOR
	o_col = i_col;
#endif
	gl_Position = vec4(i_pos * pc.scl + pc.off, 0, 1);
}
