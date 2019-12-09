in vec2 i_pos;
in vec2 i_uv;
in vec4 i_color;

out vec4 o_color;
out vec2 o_uv;
out uint o_id;

pushconstant
{
	vec2 scale;
	vec2 sdf_range;
}pc;

void main()
{
	o_color = i_color;
	o_uv = i_uv;
	o_id = gl_InstanceIndex;
	gl_Position = vec4(i_pos * pc.scale - vec2(1.0), 0, 1);
}
