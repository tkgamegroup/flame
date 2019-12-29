out vec2 o_coord;

void main()
{
	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 2.0),
		vec2(2.0, 0.0)
	};
	o_coord = vs[gl_VertexIndex];
	gl_Position = vec4(o_coord * 2.0 - 1.0, 1.0, 1.0);
}
