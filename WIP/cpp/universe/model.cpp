#include "model_private.h"

namespace flame
{
	namespace _3d
	{
		inline void ModelPrivate::add_plane(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz)
		{
			Primitive p;
			p.pt = PrimitiveTopologyPlane;
			p.p = pos;
			p.vx = vx;
			p.vz = vz;
			prims.push_back(p);
		}

		inline void ModelPrivate::add_cube(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz, float height, int side)
		{
			auto normal = -cross(vx, vz);
			normal.normalize();
			auto vy = normal * height;

			if (side & AxisPositiveX)
				add_plane(pos + vx + vy + vz, -vz, -vy);
			if (side & AxisNegativeX)
				add_plane(pos + vy, vz, -vy);
			if (side & AxisPositiveY)
				add_plane(pos + vy, vx, vz);
			if (side & AxisNegativeY)
				add_plane(pos + vz, vx, -vz);
			if (side & AxisPositiveZ)
				add_plane(pos + vy + vz, vx, -vy);
			if (side & AxisNegativeZ)
				add_plane(pos + vx + vy, -vx, -vy);
		}
	}
}

