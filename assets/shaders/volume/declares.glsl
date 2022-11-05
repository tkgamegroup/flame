// 128x128x128 cubes by default. (1 << 7) == 128
#define SHIFT 7
#define GRID_SIZE (1 << SHIFT)
#define STEP_SIZE (1.0f / float(GRID_SIZE))

struct Task
{
	uint meshlets[64];
};

float field(vec3 pos)
{
	return imageLoad(volume_datas[pc.i[0]], ivec3(pos * float(GRID_SIZE))).r;
}
