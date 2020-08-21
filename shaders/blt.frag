in vec2 i_coord;

out vec4 o_color;

sampler2D image;

pushconstant
{
	vec4 range;
}pc;

void main()
{
	o_color = texelFetch(image, ivec2(pc.range.xy + (pc.range.zw - pc.range.xy) * i_coord), 0);
}
