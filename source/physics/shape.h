#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct TriangleMesh
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static TriangleMesh* create(Device* device, graphics::Mesh* mesh);
		};

		struct HeightField
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static HeightField* create(Device* device, graphics::Image* height_map, const uvec2& blocks, uint tess_levels);
		};

		struct Shape
		{
			virtual void release() = 0;

			virtual RigidPtr get_rigid() const = 0;

			// trigger means it will not collide with others but will report when it overlay with others, default is false
			virtual void set_trigger(bool v) = 0;

			virtual void set_pose(const vec3& coord, const quat& qut) = 0;

			void* user_data;

			FLAME_PHYSICS_EXPORTS static Shape* create_box(Device* device, Material* material, const vec3& hf_ext);
			FLAME_PHYSICS_EXPORTS static Shape* create_sphere(Device* device, Material* material, float radius);
			FLAME_PHYSICS_EXPORTS static Shape* create_capsule(Device* device, Material* material, float radius, float height);
			FLAME_PHYSICS_EXPORTS static Shape* create_triangle_mesh(Device* device, Material* material, TriangleMesh* tri_mesh, float scale);
			FLAME_PHYSICS_EXPORTS static Shape* create_height_field(Device* device, Material* material, HeightField* height_field, const vec3& scale);
		};
	}
}

