#ifdef MAKE_DSL
#define TERRAIN_SET 0
#endif

struct TerrainInfo
{
	vec3 coord;

	uvec2 blocks;

	vec3 scale;
	float tess_levels;

	uint height_tex_id;
	uint normal_tex_id;
	uint color_tex_id;
};

layout(set = TERRAIN_SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[];
};
