#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;  
layout(triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec2 i_uvs[];

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;

void main(void)
{
	vec3 normal = normalize(cross(
		normalize(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz),
		normalize(gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz)
	));

	for(int i = 0; i < gl_in.length(); ++i)  
	{  
		gl_Position = gl_in[i].gl_Position;  
  
		o_uv = i_uvs[i];
		o_normal = normal;
  
		EmitVertex();  
	}  
  
	EndPrimitive();
}
