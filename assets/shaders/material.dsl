#ifdef MAKE_DSL
#define MATERIAL_SET 0
#endif

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;

	ivec4 map_indices;
};

layout (set = MATERIAL_SET, binding = 0) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (set = MATERIAL_SET, binding = 1) uniform sampler2D maps[128];
