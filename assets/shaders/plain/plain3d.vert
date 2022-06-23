layout (location = 0) in vec3 i_pos;
#ifdef USE_VERTEX_COLOR
layout (location = 1) in vec4 i_col;
#endif

#ifdef USE_VERTEX_COLOR
layout (location = 0) out vec4 o_col;
#endif

void main()
{
#ifdef USE_VERTEX_COLOR
	o_col = i_col;
#endif
	gl_Position = pc.mvp * vec4(i_pos, 1);
}
