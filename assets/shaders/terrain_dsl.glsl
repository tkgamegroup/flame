layout(set = TERRAIN_SET, binding = 0) uniform TerrianInfo
{
	vec3 coord;
	float tessellation_levels;
	uvec2 size;
	vec2 dummy1;
	vec3 extent;
}terrain_info;
