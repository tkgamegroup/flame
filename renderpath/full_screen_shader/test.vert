void main()
{
	vec2 vtxs[] = {
		vec2(0, 0),
		vec2(0, 1),
		vec2(1, 0)
	};
	gl_Position = vec4(vtxs[gl_VertexIndex], 1, 1);
}
