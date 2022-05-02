#include "plain.pll"

#ifdef USE_VERTEX_COLOR
layout (location = 0) in vec4 i_col;
#endif

layout (location = 0) out vec4 o_col;

void main()
{
	o_col = pc.col;
#ifdef USE_VERTEX_COLOR
	o_col *= i_col;
#endif
}
