#include "particle.pll"

layout(location = 0) in vec4 i_col;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in flat uint i_id;

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = i_col * texture(maps[i_id], i_uv);
}
