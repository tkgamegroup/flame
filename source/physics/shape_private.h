#pragma once

#include <flame/physics/shape.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct MaterialPrivate;
		struct RigidPrivate;

		struct ShapePrivate : Shape
		{
			RigidPrivate* rigid = nullptr;

#ifdef USE_PHYSX
			PxShape* px_shape;
#endif
			ShapePrivate(MaterialPrivate* m, ShapeType type, const ShapeDesc& desc, const Vec3f& coord, const Vec4f& quat);
			~ShapePrivate();

			void release() override { delete this; }

			void set_trigger(bool v) override;

			Rigid* get_rigid() const override { return (Rigid*)rigid; }
		};
	}
}

