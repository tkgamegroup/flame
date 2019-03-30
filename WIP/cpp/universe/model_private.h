#include <flame/3d/model.h>

#include <vector>

namespace flame
{
	namespace _3d
	{
		enum PrimitiveTopology
		{
			PrimitiveTopologyPlane,
			PrimitiveTopologyTriangle
		};

		struct Primitive
		{
			PrimitiveTopology pt;
			Vec3 p;
			Vec3 vx;
			Vec3 vz;
		};

		struct ModelPrivate : Model
		{
			std::vector<Primitive> prims;

			ModelPrivate();

			void add_plane(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz);
			void add_cube(const Vec3 &pos, const Vec3 &vx, const Vec3 &vz, float height, int side);
		};
	}
}

