void main()
{
	out_color0 = in_color;
	out_color1 = texture(images[in_id], in_uv) * in_color.a;
}
