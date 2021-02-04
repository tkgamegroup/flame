#ifdef MAKE_DSL
#define LIGHT_SET 0
#endif

struct LightSet
{
	uint directional_lights_count;
	uint directional_light_indices[7];
	uint point_lights_count;
	uint point_light_indices[1015];
};

struct LightInfo
{
	vec3 pos;
	float distance;
	vec3 color;
	
	int shadow_index;
};

layout (set = LIGHT_SET, binding = 0) buffer readonly LightSets
{
	LightSet light_sets[];
};

layout (set = LIGHT_SET, binding = 1) buffer readonly LightInfos
{
	LightInfo light_infos[];
};

layout (set = LIGHT_SET, binding = 2) buffer readonly ShadowMatrices
{
	mat4 shadow_matrices[];
};

layout (set = LIGHT_SET, binding = 3) uniform sampler2DArray directional_shadow_maps[4];
layout (set = LIGHT_SET, binding = 4) uniform samplerCube point_shadow_maps[4];
