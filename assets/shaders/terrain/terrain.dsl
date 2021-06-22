#ifdef MAKE_DSL
#define TERRAIN_SET 0
#endif

struct TerrainInfo
{
	vec3 coord;
	vec3 scale;

	uvec2 blocks;
	float tess_levels;

	uint height_map_id;
	uint material_id;
};

layout(set = TERRAIN_SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[9];
};
