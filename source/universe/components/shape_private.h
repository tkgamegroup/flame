#pragma once

#include <flame/universe/components/shape.h>

namespace flame
{
	namespace physics
	{
		struct Shape;
		struct TriangleMesh;
		struct HeightField;
	}

	struct cNodePrivate;
	struct cMeshPrivate;
	struct cTerrainPrivate;
	struct cRigidPrivate;

	struct cShapePrivate : cShape // R ~ on_*
	{
		physics::ShapeType type = physics::ShapeCube;
		vec3 size = vec3(1.f);

		bool trigger = false;

		physics::Shape* phy_shape = nullptr;
		physics::TriangleMesh* phy_triangle_mesh = nullptr;
		physics::HeightField* phy_height_field = nullptr;

		cNodePrivate* node = nullptr; // R ref
		cMeshPrivate* mesh = nullptr; // R ref optional
		cTerrainPrivate* terrain = nullptr; // R ref optional
		cRigidPrivate* rigid = nullptr; // R ref place=local|parent

		physics::ShapeType get_type() const override { return type; }
		void set_type(physics::ShapeType t) override;

		vec3 get_size() const override { return size; }
		void set_size(const vec3& s) override;

		bool get_trigger() const override { return trigger; }
		void set_trigger(bool v) override;

		void on_local_message(Message msg, void* p) override;
	};
}
