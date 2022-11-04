// 128x128x128 cubes by default. (1 << 7) == 128
#define SHIFT 7
#define GRID_SIZE (1 << SHIFT)

struct Task
{
	uint meshlets[64];
};
