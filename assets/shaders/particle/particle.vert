layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_xext;
layout(location = 2) in vec3 i_yext;
layout(location = 3) in vec4 i_uv;
layout(location = 4) in vec4 i_col;

layout(location = 0) out vec3 o_xext;
layout(location = 1) out vec3 o_yext;
layout(location = 2) out vec4 o_col;
layout(location = 3) out vec4 o_uv;
layout(location = 4) out flat uint o_id;

void main()
{
	o_xext = i_xext;
	o_yext = i_yext;
	o_col = i_col;
	o_uv = i_uv;
	o_id = gl_InstanceIndex;
	gl_Position = vec4(i_pos, 1.0);
}
