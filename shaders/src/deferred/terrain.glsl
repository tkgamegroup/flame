struct Terrain
{
	vec3 coord;
	int block_cx;
	int block_cy;
	float block_size;
	float terrain_height;
	float displacement_height;
	float tessellation_factor;
	float tiling_scale;
	uint material_count;
	uint material_index;
};

layout(binding = 3) uniform ubo_terrain_
{
	Terrain d[8];
}ubo_terrain;
