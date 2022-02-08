layout (location = 0) out vec2 o_uv;

void main()
{
	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 2.0),
		vec2(2.0, 0.0)
	};
	vec2 v = vs[gl_VertexIndex];
	o_uv = v;
	gl_Position = vec4(v * 2.0 - 1.0, 1.0, 1.0);
}
