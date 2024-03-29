layout (vertices = 4) out;
 
layout (location = 0) in vec2 i_uvs[];
 
layout (location = 0) out vec2 o_uvs[4];

uint terrain_id = pc.index & 0xffff;
#ifndef GRASS_FIELD
	uint tess_level = instance.terrains[terrain_id].tess_level;
#else
	uint tess_level = instance.terrains[terrain_id].grass_field_tess_level;
#endif

float tess_factor(vec4 p0, vec4 p1)
{
	float v = distance(camera.coord, (p0.xyz + p1.xyz) * 0.5) / camera.zFar;
	v = v * v;
	v = 1.0 - v;
	return max(v * tess_level, 1.0);
}

bool frustum_check()
{
	vec3 ext = instance.terrains[terrain_id].extent;
	float r = max(max(ext.x, ext.z), ext.y);
	vec4 p = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(camera.frustum_planes[i], p) + r < 0.0)
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
			gl_TessLevelOuter[0] = tess_factor(gl_in[3].gl_Position, gl_in[0].gl_Position);
			gl_TessLevelOuter[1] = tess_factor(gl_in[0].gl_Position, gl_in[1].gl_Position);
			gl_TessLevelOuter[2] = tess_factor(gl_in[1].gl_Position, gl_in[2].gl_Position);
			gl_TessLevelOuter[3] = tess_factor(gl_in[2].gl_Position, gl_in[3].gl_Position);

			gl_TessLevelInner[0] = mix(gl_TessLevelOuter[1], gl_TessLevelOuter[3], 0.5);
			gl_TessLevelInner[1] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[2], 0.5);
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	o_uvs[gl_InvocationID] = i_uvs[gl_InvocationID];
} 
