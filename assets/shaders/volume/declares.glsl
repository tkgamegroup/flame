// 128x128x128 cubes by default. (1 << 7) == 128
#define SHIFT 7
#define GRID_SIZE (1 << SHIFT)
#define STEP_SIZE (1.0f / float(GRID_SIZE))

struct Task
{
	uint meshlets[64];
};

uint vol_id = pc.index >> 16;
vec3 blocks = vec3(instance.volumes[vol_id].blocks);
vec3 block_sz = vec3(1.0) / blocks;
vec3 block_off = block_sz * pc.offset;

float field(vec3 pos)
{
	return texture(volume_datas[vol_id], pos * block_sz + block_off).r * 2.0 - 1.0;
}
