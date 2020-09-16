struct PointLightInfo
{
	vec3 color;
	int dummy0;
	vec3 coord;

	int shadow_map_index;
};

struct PointLightIndices
{
	uint count;
	uint indices[1023];
};

layout (set = 2, binding = 0) buffer readonly PointLightInfos
{
	PointLightInfo point_light_infos[];
};

layout (set = 2, binding = 1) buffer readonly PointLightIndicesList
{
	PointLightIndices point_light_indices_list[];
};

layout (set = 2, binding = 2) uniform samplerCube point_light_shadow_maps[4];
