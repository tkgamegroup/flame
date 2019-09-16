void main()
{
	vec2 vtxs[] = {
		vec2(-1, -1),
		vec2(-1, 3),
		vec2(3, -1)
	};
	gl_Position = vec4(vtxs[gl_VertexIndex], 1, 1);
}
