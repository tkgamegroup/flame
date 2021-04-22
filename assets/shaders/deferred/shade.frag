#include "shade.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	vec4 alb_met = texture(img_alb_met, i_uv);
	vec4 nor_rou = texture(img_nor_rou, i_uv);
	o_color = vec4(alb_met.rgb, 1.0);
}
