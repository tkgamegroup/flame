struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;

	int map_indices[8];
};

layout (set = SET, binding = 0) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (set = SET, binding = 1) uniform sampler2D maps[128];
