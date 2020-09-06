#pragma once

#include <flame/universe/components/shape.h>

namespace flame
{
	namespace physics
	{
		struct Shape;
	}

	struct cNodePrivate;
	struct cObjectPrivate;
	struct cRigidPrivate;

	struct cShapePrivate : cShape // R ~ on_*
	{
		physics::ShapeType type = physics::ShapeBox;

		physics::Shape* phy_shape;

		cNodePrivate* node = nullptr; // R ref
		cRigidPrivate* rigid = nullptr; // R ref place=parent

		physics::ShapeType get_type() const override { return type; }
		void set_type(physics::ShapeType t) override;

		void on_gain_rigid();
		void on_lost_rigid();
	};
}
