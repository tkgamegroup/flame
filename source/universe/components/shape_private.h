#pragma once

#include <flame/universe/components/shape.h>

namespace flame
{
	namespace physics
	{
		struct Shape;
	}

	struct cNodePrivate;
	struct cRigidPrivate;

	struct cShapePrivate : cShape // R ~ on_*
	{
		physics::Shape* shape = nullptr;

		cNodePrivate* node = nullptr; // R ref
		cRigidPrivate* rigid = nullptr; // R ref

		void on_gain_rigid();
		void on_lost_rigid();
	};
}
