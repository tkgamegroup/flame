void main()
{
	out_color = in_color;
	out_uv = in_uv;
	out_id = gl_InstanceIndex;
	gl_Position = vec4(in_pos * pc.scale - vec2(1.0), 0, 1);
}
