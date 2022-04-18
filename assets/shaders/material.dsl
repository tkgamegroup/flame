struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;
	int opaque;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) uniform MaterialMisc
{
	int black_map_id;
	int white_map_id;
	int random_map_id;
}material_misc;

layout (set = SET, binding = 1) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (set = SET, binding = 2) uniform sampler2D material_maps[128];
