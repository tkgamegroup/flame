layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) in flat uint i_id[];
layout(location = 1) in flat uint i_matid[];
layout(location = 2) in vec2 i_uv[];
layout(location = 3) in vec3 i_normal[];
layout(location = 4) in vec3 i_tangent[];
layout(location = 5) in vec3 i_coordw[];

layout(location = 0) out vec3 o_normal;

void main()
{
	vec3 pos = (gl_in[0].gl_Position.xyz + 
		gl_in[1].gl_Position.xyz +
		gl_in[2].gl_Position.xyz) / 3.0;

	o_normal = vec3(0, 0, 1);
	gl_Position = scene.proj_view * vec4(pos + vec3(-0.1, 0, 0), 1.0);
	EmitVertex();
	
	o_normal = vec3(0, 0, 1);
	gl_Position = scene.proj_view * vec4(pos + vec3(+0.1, 0, 0), 1.0);
	EmitVertex();
	
	o_normal = vec3(0, 0, 1);
	gl_Position = scene.proj_view * vec4(pos + vec3(0, 1, 0), 1.0);
	EmitVertex();

	EndPrimitive();
}
