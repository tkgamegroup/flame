struct TerrainInfo
{
	vec3 coord;
	float dummy1;

	uvec2 size;
	uint height_tex_id;
	uint color_tex_id;

	vec3 extent;
	float tess_levels;
};

layout(set = TERRAIN_SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[];
};
