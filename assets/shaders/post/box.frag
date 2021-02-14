#include "post.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = texture(image, i_uv + vec2(-pc.pxsz.x, -pc.pxsz.y)) * 0.25 +
		texture(image, i_uv + vec2(+pc.pxsz.x, -pc.pxsz.y)) * 0.25 +
		texture(image, i_uv + vec2(+pc.pxsz.x, +pc.pxsz.y)) * 0.25 +
		texture(image, i_uv + vec2(-pc.pxsz.x, +pc.pxsz.y)) * 0.25;
}
