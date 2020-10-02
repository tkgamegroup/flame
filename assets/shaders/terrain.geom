#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define RENDER_DATA_SET 0
#define TERRAIN_SET 3

#include "render_data_dsl.glsl"
#include "terrain_dsl.glsl"

layout(triangles) in;  
layout(triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec2 i_uvs[];
layout (location = 1) in vec3 i_normals[];
layout (location = 2) in vec4 i_debug[];


layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out vec4 o_debug;

void main(void)
{
	for(int i = 0; i < gl_in.length(); ++i)  
	{  
		gl_Position = gl_in[i].gl_Position;  
  
		o_uv = i_uvs[i];
		o_normal = i_normals[i];
		o_debug = i_debug[i];
  
		EmitVertex();  
	}  
  
	EndPrimitive();
}
