#ifdef MAKE_DSL
#define LIGHT_SET 0
#endif

struct GridLightRef
{
	uint dir_count;
	uint dir_indices[7];
	uint pt_count;
	uint pt_indices[1015];
};

struct LightInfo
{
	vec3 pos;
	float distance;
	vec3 color;
	
	int shadow_index;
};

layout (set = LIGHT_SET, binding = 0) buffer readonly GridLights
{
	GridLightRef grid_lights[20000];
};

layout (set = LIGHT_SET, binding = 1) buffer readonly LightData
{
	LightInfo light_infos[65536];
	mat4 dir_mats[16];
	mat4 pt_mats[24];
}light_data;

layout (set = LIGHT_SET, binding = 2) uniform sampler2DArray	dir_maps[4];
layout (set = LIGHT_SET, binding = 3) uniform samplerCube		pt_maps[4];
