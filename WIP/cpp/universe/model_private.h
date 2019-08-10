#include <flame/3d/model.h>

#include <vector>

namespace flame
{
	namespace _3d
	{
		struct ModelPrivate : Model
		{
			ModelPrivate();

			void add_plane(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz);
			void add_cube(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz, float height, int side);
		};
	}
}

