struct TerrainInfo
{
	vec3 coord;
	vec3 extent;

	uvec2 blocks;
	float tess_levels;

	uint height_map_id;
	uint normal_map_id;
	uint tangent_map_id;
	uint material_id;
};

layout(set = SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[9];
};