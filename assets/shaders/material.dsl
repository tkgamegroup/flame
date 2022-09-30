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
	MaterialInfo infos[128];
}material;

layout (set = SET, binding = 1) uniform sampler2D material_maps[128];
