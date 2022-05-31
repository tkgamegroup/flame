struct LightGrid
{
	uint offset;
	uint count;
};

struct LightInfo
{
	vec3 pos;
	uint type;
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

layout (set = SET, binding = 0) buffer readonly LightIndexs
{
	uint light_indexs[32400 * 16];
};

layout (set = SET, binding = 1) buffer readonly LightGrids
{
	LightGrid light_grids[32400];
};

layout (set = SET, binding = 2) buffer readonly LightInfos
{
	LightInfo light_infos[1024];
};

layout (set = SET, binding = 3) buffer readonly DirShadows
{
	DirShadow dir_shadows[4];
};

layout(set = SET, binding = 4) buffer readonly PtShadows
{
	PtShadow pt_shadows[4];
};

layout (set = SET, binding = 5) uniform sampler2DArray	dir_shadow_maps[4];
layout (set = SET, binding = 6) uniform samplerCube		pt_shadow_maps[4];

layout(set = SET, binding = 7) uniform samplerCube sky_map;
layout(set = SET, binding = 8) uniform samplerCube sky_irr_map;
layout(set = SET, binding = 9) uniform samplerCube sky_rad_map;
layout(set = SET, binding = 10) uniform sampler2D brdf_map;
