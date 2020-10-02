struct TerrainInfo
{
	vec3 coord;
	float dummy1;

	uvec2 size;
	uint height_tex_id;
	float tess_levels;

	vec3 extent;
	float dummy2;
};

layout(set = TERRAIN_SET, binding = 0) uniform TerrainInfos
{
	TerrainInfo terrain_infos[];
};
