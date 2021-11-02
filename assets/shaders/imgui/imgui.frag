#include "imgui.pll"

layout (location = 0) in vec4 i_color;
layout (location = 1) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = i_color * texture(image, i_uv);
}
