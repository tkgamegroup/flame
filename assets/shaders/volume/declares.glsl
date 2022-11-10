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
	return texture(volume_datas[pc.i[0]], pos + STEP_SIZE * 0.25).r * 2.0 - 1.0;
}
