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

		physics::Shape* shape = nullptr;

		cNodePrivate* node = nullptr; // R ref
		cObjectPrivate* object = nullptr; // R ref
		cRigidPrivate* rigid = nullptr; // R ref

		physics::ShapeType get_type() const override { return type; }
		void set_type(physics::ShapeType t) override;

		void make_shape();

		void on_gain_rigid();
		void on_lost_rigid();

		void on_local_data_changed(Component* t, uint64 h) override;
	};
}
