#include "terrain.glsl"

layout (vertices = 4) out;
 
layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV[4];

float screen_space_tess_factor(vec4 p0, vec4 p1)
{
	float r = distance(p0, p1) / 2.0;

	vec4 v0 = ubo_terrain.view_matrix * (0.5 * (p0 + p1));

	vec4 c0 = (ubo_terrain.proj_matrix * (v0 - vec4(r, vec3(0.0))));
	vec4 c1 = (ubo_terrain.proj_matrix * (v0 + vec4(r, vec3(0.0))));

	c0 /= c0.w;
	c1 /= c1.w;

	c0.xy *= ubo_terrain.resolution;
	c1.xy *= ubo_terrain.resolution;
	
	return clamp(distance(c0, c1) / ubo_terrain.size * 
		ubo_terrain.tessellation_factor, 1.0, 64.0);
}

bool frustum_check()
{
	const float r = max(ubo_terrain.size, ubo_terrain.height);
	vec4 pos = (gl_in[0].gl_Position + gl_in[1].gl_Position + 
		gl_in[2].gl_Position + gl_in[3].gl_Position) / 4;
	pos = ubo_terrain.proj_matrix * ubo_terrain.view_matrix * pos;
	pos /= pos.w;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(pos, ubo_terrain.frustum_planes[i]) + r < 0.0)
			return false;
	}
	return true;
}

void main()
{
	outUV[gl_InvocationID] = inUV[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		if (!frustum_check())
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else
		{
			gl_TessLevelOuter[0] = screen_space_tess_factor(gl_in[3].gl_Position, gl_in[0].gl_Position);
			gl_TessLevelOuter[1] = screen_space_tess_factor(gl_in[0].gl_Position, gl_in[1].gl_Position);
			gl_TessLevelOuter[2] = screen_space_tess_factor(gl_in[1].gl_Position, gl_in[2].gl_Position);
			gl_TessLevelOuter[3] = screen_space_tess_factor(gl_in[2].gl_Position, gl_in[3].gl_Position);
			gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
			gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 
