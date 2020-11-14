#ifdef MAKE_DSL
#define MATERIAL_SET 0
#endif

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;

	int color_map_index;
	int metallic_roughness_ao_map_index;
	int normal_height_map_index;
};

layout (set = MATERIAL_SET, binding = 0) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[];
};

layout (set = MATERIAL_SET, binding = 1) uniform sampler2D maps[128];
