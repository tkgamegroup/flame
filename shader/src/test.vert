void main()
{
	gl_Position = vec4(vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2) * 2.0 - 1.0, 1, 1);
}
