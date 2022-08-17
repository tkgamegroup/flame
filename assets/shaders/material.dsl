struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	int opaque;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) buffer readonly Material
{
	int black_map_id;
	int white_map_id;
	int random_map_id;
	MaterialInfo infos[128];
}material;

layout (set = SET, binding = 1) uniform sampler2D material_maps[128];
