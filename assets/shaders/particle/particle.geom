layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec3 i_xext[];
layout(location = 1) in vec3 i_yext[];
layout(location = 2) in vec4 i_col[];
layout(location = 3) in vec4 i_uv[];
layout(location = 4) in flat uint i_id[];

layout(location = 0) out vec4 o_col;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out flat uint o_id;

void main()
{
	vec3 pos = vec3(gl_in[0].gl_Position);

	o_col = i_col[0];
	o_uv = i_uv[0].xw;
	o_id = i_id[0];
	gl_Position = camera.proj_view * vec4(pos - i_xext[0] - i_yext[0], 1.0);
	EmitVertex();

	o_col = i_col[0];
	o_uv = i_uv[0].zw;
	o_id = i_id[0];
	gl_Position = camera.proj_view * vec4(pos + i_xext[0] - i_yext[0], 1.0);
	EmitVertex();

	o_col = i_col[0];
	o_uv = i_uv[0].xy;
	o_id = i_id[0];
	gl_Position = camera.proj_view * vec4(pos - i_xext[0] + i_yext[0], 1.0);
	EmitVertex();

	o_col = i_col[0];
	o_uv = i_uv[0].zy;
	o_id = i_id[0];
	gl_Position = camera.proj_view * vec4(pos + i_xext[0] + i_yext[0], 1.0);
	EmitVertex();

	EndPrimitive();
}
