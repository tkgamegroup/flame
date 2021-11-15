#ifdef MAKE_DSL
#define WATER_SET 0
#endif

struct WaterInfo
{
	vec3 coord;
	vec2 extent;

	uint material_id;
};

layout (set = WATER_SET, binding = 0) buffer readonly WaterInfos
{
	WaterInfo water_infos[16];
};

layout (set = WATER_SET, binding = 1) uniform sampler2D img_depth;
