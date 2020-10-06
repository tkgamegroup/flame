#pragma once

#include <flame/universe/components/shape.h>

namespace flame
{
	namespace physics
	{
		struct Shape;
	}

	struct cNodePrivate;
	struct cMeshPrivate;
	struct cTerrainPrivate;
	struct cRigidPrivate;

	struct cShapePrivate : cShape // R ~ on_*
	{
		physics::ShapeType type = physics::ShapeCube;
		Vec3f size = Vec3f(1.f);

		bool trigger = false;

		physics::Shape* phy_shape;

		cNodePrivate* node = nullptr; // R ref
		cMeshPrivate* mesh = nullptr; // R ref optional
		cTerrainPrivate* terrain = nullptr; // R ref optional
		cRigidPrivate* rigid = nullptr; // R ref place=local|parent

		physics::ShapeType get_type() const override { return type; }
		void set_type(physics::ShapeType t) override;

		Vec3f get_size() const override { return size; }
		void set_size(const Vec3f& s) override;

		bool get_trigger() const override { return trigger; }
		void set_trigger(bool v) override;

		void on_local_message(Message msg, void* p) override;
	};
}
