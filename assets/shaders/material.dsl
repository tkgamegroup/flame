struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	vec4 emissive;
	float tiling;
	float normal_map_strength;
	float emissive_map_strength;
	uint flags;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) buffer readonly Material
{
	vec4 vars[128];
	MaterialInfo infos[128];
}material;

layout (set = SET, binding = 1) uniform sampler2D material_maps[128];

vec4 mat_var(uint id)
{
	return material.vars[id];
}

vec4 sample_map(uint id, in vec2 uv)
{
	return texture(material_maps[id], uv);
}
