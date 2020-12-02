#ifdef MAKE_DSL
#define LIGHT_SET 0
#endif

struct LightIndices
{
	uint directional_lights_count;
	uint point_lights_count;
	uint point_light_indices[1022];
};

struct DirectionalLightInfo
{
	vec3 dir;
	float distance;
	vec3 color;
	
	int shadow_map_index;
	mat4 shadow_matrices[4];
};

struct PointLightInfo
{
	vec3 coord;
	float distance;
	vec3 color;
	int shadow_map_index;
};

layout (set = LIGHT_SET, binding = 0) buffer readonly LightIndicesList
{
	LightIndices light_indices_list[];
};

layout (set = LIGHT_SET, binding = 1) buffer readonly DirectionalLightInfos
{
	DirectionalLightInfo directional_light_infos[];
};

layout (set = LIGHT_SET, binding = 2) buffer readonly PointLightInfos
{
	PointLightInfo point_light_infos[];
};

layout (set = LIGHT_SET, binding = 3) uniform sampler2DArray directional_shadow_maps[4];
layout (set = LIGHT_SET, binding = 4) uniform samplerCube point_shadow_maps[4];
