#ifdef MAKE_DSL
#define LIGHT_SET 0
#endif

struct TileLights
{
	uint dir_count;
	uint dir_indices[7];
	uint pt_count;
	uint pt_indices[1015];
};

struct LightInfo
{
	vec3 pos;
	vec3 color;
	
	int shadow_index;
};

struct DirShadow
{
	mat4 mats[4];
	float splits[4];
	float far;
};

struct PtShadow
{
	mat4 mats[6];
	float near;
	float far;
};

layout (set = LIGHT_SET, binding = 0) buffer readonly TileLightsMap
{
	TileLights tile_lights[20000];
};

layout (set = LIGHT_SET, binding = 1) buffer readonly LightInfos
{
	LightInfo light_infos[65536];
};

layout (set = LIGHT_SET, binding = 2) buffer readonly DirShadows
{
	DirShadow dir_shadows[4];
};

layout(set = LIGHT_SET, binding = 3) buffer readonly PtShadows
{
	PtShadow pt_shadows[4];
};

layout (set = LIGHT_SET, binding = 4) uniform sampler2DArray	dir_shadow_maps[4];
layout (set = LIGHT_SET, binding = 5) uniform samplerCube		pt_shadow_maps[4];

layout(set = LIGHT_SET, binding = 6) uniform samplerCube sky_box;
layout(set = LIGHT_SET, binding = 7) uniform samplerCube sky_irr;
layout(set = LIGHT_SET, binding = 8) uniform samplerCube sky_rad;
layout(set = LIGHT_SET, binding = 9) uniform sampler2D sky_lut;
