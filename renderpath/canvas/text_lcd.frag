void main()
{
	vec4 v = texture(images[in_id], in_uv);
	out_color0 = in_color;
	out_color1 = v * in_color.a;
}
