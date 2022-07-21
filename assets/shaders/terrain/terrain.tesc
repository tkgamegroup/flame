layout (vertices = 4) out;
 
layout (location = 0) in flat uint i_ids[];
layout (location = 1) in flat uint i_matids[];
layout (location = 2) in	  vec2 i_uvs[];
 
layout (location = 0) out flat	uint o_ids[4];
layout (location = 1) out flat	uint o_matids[4];
layout (location = 2) out		vec2 o_uvs[4];

uint id;
uint tess_level;

float tess_factor(vec4 p0, vec4 p1)
{
	float v = distance(scene.camera_coord, (p0.xyz + p1.xyz) * 0.5) / scene.zFar;
	v = v * v;
	v = 1.0 - v;
#endif
	return max(v * tess_level, 1.0);
}

bool frustum_check()
{
	vec3 ext = terrain_instances[id].extent;
	float r = max(max(ext.x, ext.z), ext.y);
	vec4 p = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(scene.frustum_planes[i], p) + r < 0.0)
			return false;
	}
	return true;
}

void main()
{
	id = i_ids[0];
#ifndef GRASS_FIELD
	tess_level = terrain_instances[id].tess_level;
#else
	tess_level = grass_field_instances[terrain_instances[id].grass_field_id].tess_level;

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

	o_ids[gl_InvocationID] = i_ids[gl_InvocationID];
	o_matids[gl_InvocationID] = i_matids[gl_InvocationID];
	o_uvs[gl_InvocationID] = i_uvs[gl_InvocationID];
} 
