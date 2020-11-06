#pragma once

#include <flame/physics/shape.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate;
		struct MaterialPrivate;
		struct RigidPrivate;

		struct TriangleMeshPrivate : TriangleMesh
		{
#ifdef USE_PHYSX
			PxTriangleMesh* px_triangle_mesh;
#endif

			TriangleMeshPrivate(DevicePrivate* device, graphics::Mesh* mesh);
			~TriangleMeshPrivate();

			void release() override { delete this; }
		};

		struct HeightFieldPrivate : HeightField
		{
			uint tess_levels;

#ifdef USE_PHYSX
			PxHeightField* px_height_field;
#endif

			HeightFieldPrivate(DevicePrivate* device, graphics::Image* height_map, const Vec2u& blocks, uint tess_levels);
			~HeightFieldPrivate();

			void release() override { delete this; }
		};

		struct ShapePrivate : Shape
		{
			RigidPrivate* rigid = nullptr; 

#ifdef USE_PHYSX
			PxShape* px_shape;
#endif

			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, const Vec3f& hf_ext);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, float radius, float height);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, TriangleMeshPrivate* tri_mesh, float scale);
			ShapePrivate(DevicePrivate* device, MaterialPrivate* material, HeightFieldPrivate* height_field, const Vec3f& scale);
			~ShapePrivate();

			void release() override { delete this; }

			Rigid* get_rigid() const override { return (Rigid*)rigid; }

			void set_trigger(bool v) override;

			void set_pose(const Vec3f& coord, const Vec4f& quat) override;
		};
	}
}

