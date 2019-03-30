#include <flame/3d/3d.h>

#include <flame/math.h>

namespace flame
{
	namespace _3d
	{
		struct Model
		{
			FLAME_3D_EXPORTS void add_plane(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz);
			FLAME_3D_EXPORTS void add_cube(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz, float height, int side = AxisPositiveX | AxisNegativeX | AxisPositiveY | AxisNegativeY | AxisPositiveZ | AxisNegativeZ);

			FLAME_3D_EXPORTS static Model *create();
			FLAME_3D_EXPORTS static void destroy(Model *m);
		};
	}
}

