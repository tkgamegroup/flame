struct Water
{
	vec3 coord;
	int block_cx;
	int block_cy;
	float block_size;
	float height;
	float tessellation_factor;
	float tiling_scale;
	float mapDimension;
};

layout(binding = 5) uniform ubo_water_
{
	Water d[8];
}ubo_water;
