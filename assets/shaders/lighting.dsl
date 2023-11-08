struct DirLight
{
	vec3 dir;
	vec3 color;
	int shadow_index;
};

struct PtLight
{
	vec3 pos;
	vec3 color;
	int shadow_index;
};

struct DirShadow
{
	mat4 mats[4];
	vec4 splits;
	float far;
};

struct PtShadow
{
	mat4 mats[6];
	float near;
	float far;
};

layout (set = SET, binding = 0) buffer readonly Lighting
{
	float sky_intensity;
	float sky_rad_levels;
	float esm_factor;
	float shadow_bleeding_reduction;
	float shadow_darkening;
	vec3 fog_color;
	uint ssr_enable;
	float ssr_thickness;
	float ssr_max_distance;
	int ssr_max_steps;
	int ssr_binary_search_steps;
	
	DirLight dir_lights[4];
	uint dir_lights_count;
	uint dir_lights_list[4];
	DirShadow dir_shadows[4];

	PtLight pt_lights[1024];
	uint pt_lights_count;
	uint pt_lights_list[1024];
	PtShadow pt_shadows[4];
}lighting;

layout (set = SET, binding = 1) uniform sampler2DArray	dir_shadow_maps[4];
layout (set = SET, binding = 2) uniform samplerCube		pt_shadow_maps[4];

layout(set = SET, binding = 3) uniform samplerCube sky_map;
layout(set = SET, binding = 4) uniform samplerCube sky_irr_map;
layout(set = SET, binding = 5) uniform samplerCube sky_rad_map;
layout(set = SET, binding = 6) uniform sampler2D brdf_map;
