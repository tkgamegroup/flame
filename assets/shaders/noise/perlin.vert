layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_uv;
#ifdef USE_VERTEX_VAL_BASE
layout (location = 2) in float i_val_base;
#endif

layout (location = 0) out vec2 o_uv;
#ifdef USE_VERTEX_VAL_BASE
layout (location = 1) out float o_val_base;
#endif

void main()
{
	o_uv = i_uv;
#ifdef USE_VERTEX_VAL_BASE
	o_val_base = i_val_base;
#endif
	gl_Position = vec4(i_pos, 0.0, 1.0);
}
