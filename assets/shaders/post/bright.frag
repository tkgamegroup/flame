#include "post.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	vec3 color = texture(image, i_uv).rgb;
	float brightness = max(color.r, max(color.g, color.b));
	float contribution = max(0.0, brightness - 1.0);
	contribution /= max(brightness, 0.00001);
	o_color = vec4(color * contribution, 1.0);
}
