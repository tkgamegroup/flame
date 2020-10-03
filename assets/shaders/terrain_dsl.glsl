struct TerrainInfo
{
	vec3 coord;
	float dummy1;

	uvec2 size;
	uint height_tex_id;
	float tess_levels;

	vec3 extent;
	int blend_tex_id;
	
	ivec4 color_tex_ids;
};

layout(set = TERRAIN_SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[];
};
