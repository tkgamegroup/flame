#pragma once

#include "shape.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct TriangleMeshPrivate : TriangleMesh
		{
			DevicePrivate* device;

			std::unique_ptr<PxTriangleMesh> px_triangle_mesh;

			TriangleMeshPrivate(DevicePrivate* device, graphics::Mesh* mesh);

			void release() override { delete this; }
		};

		struct HeightFieldPrivate : HeightField
		{
			DevicePrivate* device;

			uvec2 blocks;
			uint tess_levels;

			std::unique_ptr<PxHeightField> px_height_field;

			HeightFieldPrivate(DevicePrivate* device, graphics::Image* height_map, const uvec2& blocks, uint tess_levels);

			void release() override { delete this; }
		};

		struct ShapePrivate : Shape
		{
			DevicePrivate* device;

			ShapeType type;
			RigidPrivate* rigid = nullptr; 

			std::unique_ptr<PxShape> px_shape;

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

