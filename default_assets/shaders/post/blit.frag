#include "post.pll"

layout (location = 0) in vec2 i_uv;

#ifndef DEPTH
layout (location = 0) out vec4 o_color;
#endif

void main()
{
#ifdef DEPTH
	gl_FragDepth = texture(image, i_uv).r;
#else
	o_color = texture(image, i_uv);
#endif
}
