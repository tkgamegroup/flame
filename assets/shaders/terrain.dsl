#ifdef MAKE_DSL
#define TERRAIN_SET 0
#endif

struct TerrainInfo
{
	vec3 coord;
	float dummy1;

	uvec2 blocks;
	vec2 dummy2;

	vec3 scale;
	float tess_levels;

	uint height_tex_id;
	uint normal_tex_id;
	uint color_tex_id;
	float dummy3;
};

layout(set = TERRAIN_SET, binding = 0) buffer readonly TerrainInfos
{
	TerrainInfo terrain_infos[];
};
