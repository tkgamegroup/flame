struct WaterInfo
{
	vec3 coord;
	vec2 extent;

	uint material_id;
};

layout (set = SET, binding = 0) buffer readonly WaterInfos
{
	WaterInfo water_infos[16];
};

layout (set = SET, binding = 1) uniform sampler2D img_depth;
