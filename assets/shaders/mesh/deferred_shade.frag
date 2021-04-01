#include "deferred_shade.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	vec4 sample0 = texture(image0, i_uv);
	vec4 sample1 = texture(image1, i_uv);
	o_color = vec4(sample0.rgb, 1.0);
}
