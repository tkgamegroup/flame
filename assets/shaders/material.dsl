struct MaterialInfo
{
	vec4 color;
	int opaque;
	float metallic;
	float roughness;
	float alpha_test;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (set = SET, binding = 1) uniform sampler2D material_maps[128];
