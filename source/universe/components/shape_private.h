#pragma once
#ifdef USE_PHYSICS_MODULE

#include "shape.h"

namespace flame
{
	struct cShapePrivate : cShape
	{
		physics::ShapeType type = physics::ShapeBox;
		vec3 size = vec3(0.5f);
		float static_friction = 0.2f;
		float dynamic_friction = 0.2f;
		float restitution = 0.3f;

		bool trigger = false;

		physics::Shape* phy_shape = nullptr;
		physics::TriangleMesh* phy_triangle_mesh = nullptr;
		physics::HeightField* phy_height_field = nullptr;

		cNodePrivate* node = nullptr;
		cMeshPrivate* c_mesh = nullptr;
		cTerrainPrivate* c_terrain = nullptr;
		cRigidPrivate* rigid = nullptr;

		physics::ShapeType get_type() const override { return type; }
		void set_type(physics::ShapeType t) override;

		vec3 get_size() const override { return size; }
		void set_size(const vec3& s) override;

		float get_static_friction() const override { return static_friction; }
		void set_static_friction(float v) override;

		float get_dynamic_friction() const override { return dynamic_friction; }
		void set_dynamic_friction(float v) override;

		float get_restitution() const override { return restitution; }
		void set_restitution(float v) override;

		bool get_trigger() const override { return trigger; }
		void set_trigger(bool v) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}

#endif
