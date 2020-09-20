struct LightIndices
{
	uint directional_lights_count;
	uint point_lights_count;
	uint point_light_indices[1022];
};

struct DirectionalLightInfo
{
	vec3 dir;
	int dummy1;
	vec3 side;
	int dummy2;
	vec3 up;
	int dummy3;
	vec3 color;
	int dummy4;

	int shadow_map_index;
	float shadow_distance;
	ivec2 dummy5;
	mat4 shadow_matrices[4];
};

struct PointLightInfo
{
	vec3 coord;
	int dummy1;
	vec3 color;
	int dummy2;

	int shadow_map_index;
	float shadow_distance;
	int dummy3[2];
};

layout (set = 2, binding = 0) buffer readonly LightIndicesList
{
	LightIndices light_indices_list[];
};

layout (set = 2, binding = 1) buffer readonly DirectionalLightInfos
{
	DirectionalLightInfo directional_light_infos[];
};

layout (set = 2, binding = 2) buffer readonly PointLightInfos
{
	PointLightInfo point_light_infos[];
};

layout (set = 2, binding = 3) uniform sampler2DArray directional_light_shadow_maps[4];
layout (set = 2, binding = 4) uniform samplerCube point_light_shadow_maps[4];
