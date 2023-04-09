#include "plain2d.vi"

#ifdef USE_VERTEX_COLOR
layout (location = 0) out vec4 o_col;
#endif

void main()
{
#ifdef USE_VERTEX_COLOR
	o_col = i_col;
#endif
	gl_Position = vec4(i_pos * pc.scl + pc.off, 0, 1);
}
