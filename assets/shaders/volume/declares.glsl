// 32x32x32 cubes by default. (1 << 5) == 32
#define SHIFT 5
#define GRID_SIZE (1 << SHIFT)
#define STEP_SIZE (1.0f / float(GRID_SIZE))

struct Task
{
	uint meshlets[64];
};

float field(vec3 pos)
{
	return texture(volume_datas[pc.i[0]], pos).r;
}
