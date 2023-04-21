// 128x128x128 cubes by default. (1 << 7) == 128
#define SHIFT 7
#define GRID_SIZE (1 << SHIFT)
#define STEP_SIZE (1.0f / float(GRID_SIZE))

struct Task
{
	uint meshlets[64];
};

#ifndef CUSTOM_INPUT
uint volume_id = pc.index & 0xffff;
vec3 extent = instance.volumes[volume_id].extent;
vec3 blocks = vec3(instance.volumes[volume_id].blocks);
#define DATA_MAP volume_data_maps[volume_id]
#else
vec3 extent = _extent;
vec3 blocks = _blocks;
#endif

vec3 block_sz = vec3(1.0) / blocks;
vec3 block_off = block_sz * pc.offset;

float field(vec3 pos)
{
	return texture(DATA_MAP, pos * block_sz + block_off).r * 2.0 - 1.0;
}
