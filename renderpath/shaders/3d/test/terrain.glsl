layout(binding = 0) uniform ubo_terrain_
{
	vec3 coord;
	float dummy0;
	ivec2 count;
	float size;
	float height;
	vec2 resolution;
	float tessellation_factor;
	float dummy1;
	mat4 view_matrix;
	mat4 proj_matrix;
	vec4 frustum_planes[6];
}ubo_terrain;
