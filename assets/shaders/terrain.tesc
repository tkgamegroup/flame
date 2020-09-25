#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define RENDER_DATA_SET 2
#define TERRAIN_SET 3

#include "render_data_dsl.glsl"
#include "terrain_dsl.glsl"

layout (vertices = 4) out;
 
layout (location = 0) in vec2 i_uvs[];
 
layout (location = 0) out vec2 o_uvs[4];
layout (location = 1) out vec4 o_debug[4];

float screen_space_tessellation_factor(vec4 p0, vec4 p1)
{
	float v = distance(render_data.camera_coord, (p0.xyz + p1.xyz) * 0.5) / render_data.zFar;
	v = v * v;
	v = 1.0 - v;
	return max(v * terrain_info.tessellation_levels, 1.0);
}

bool frustum_check()
{
	float r = max(max(terrain_info.extent.x, terrain_info.extent.z), terrain_info.extent.y);
	vec4 p = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(p, render_data.frustum_planes[i]) > r)
			return false;
	}
	return true;
}

void main()
{
	if (gl_InvocationID == 0)
	{
		if (!frustum_check())
		{
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;

			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
		}
		else
		{
			gl_TessLevelOuter[0] = screen_space_tessellation_factor(gl_in[3].gl_Position, gl_in[0].gl_Position);
			gl_TessLevelOuter[1] = screen_space_tessellation_factor(gl_in[0].gl_Position, gl_in[1].gl_Position);
			gl_TessLevelOuter[2] = screen_space_tessellation_factor(gl_in[1].gl_Position, gl_in[2].gl_Position);
			gl_TessLevelOuter[3] = screen_space_tessellation_factor(gl_in[2].gl_Position, gl_in[3].gl_Position);

			gl_TessLevelInner[0] = mix(gl_TessLevelOuter[1], gl_TessLevelOuter[3], 0.5);
			gl_TessLevelInner[1] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[2], 0.5);
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	o_uvs[gl_InvocationID] = i_uvs[gl_InvocationID];
} 
