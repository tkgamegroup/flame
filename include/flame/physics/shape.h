#pragma once

#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Rigid;
		struct Material;

		struct Shape
		{
			virtual void release() = 0;

			// trigger means it will not collide with others but will report when it overlay with others, default is false
			virtual void set_trigger(bool v) = 0;

			virtual Rigid* get_rigid() const = 0;

			void* user_data;

			FLAME_PHYSICS_EXPORTS static Shape* create(Device* device, Material* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord = Vec3f(0.f), const Vec4f& quat = Vec4f(0.f, 0.f, 0.f, 1.f));
		};
	}
}

