#include "element.pll"

layout (location = 0) in vec2 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec4 i_color;

layout (location = 0) out vec4 o_color;
layout (location = 1) out vec2 o_uv;

void main()
{
	o_color = i_color;
	o_uv = i_uv;
	gl_Position = vec4(i_position * pc.scale - vec2(1.0), 0, 1);
}
