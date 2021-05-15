#pragma once

#include "shape.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct TriangleMeshPrivate : TriangleMesh
		{
#ifdef USE_PHYSX
			UniPtr<PxTriangleMesh> px_triangle_mesh;
#endif

			TriangleMeshPrivate(DevicePrivate* device, graphics::Mesh* mesh);

			void release() override { delete this; }
		};

		struct HeightFieldPrivate : HeightField
		{
			uint tess_levels;

#ifdef USE_PHYSX
			UniPtr<PxHeightField> px_height_field;
#endif

			HeightFieldPrivate(DevicePrivate* device, graphics::Image* height_map, const uvec2& blocks, uint tess_levels);

			void release() override { delete this; }
		};

		struct ShapePrivate : Shape
		{
			RigidPrivate* rigid = nullptr; 

#ifdef USE_PHYSX
			UniPtr<PxShape> px_shape;
#endif

			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, const vec3& hf_ext);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius, float height);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, TriangleMeshPrivate* tri_mesh, float scale);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, HeightFieldPrivate* height_field, const vec3& scale);

			void release() override { delete this; }

			RigidPtr get_rigid() const override { return rigid; }

			void set_trigger(bool v) override;

			void set_pose(const vec3& coord, const quat& qut) override;
		};
	}
}

