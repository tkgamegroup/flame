#pragma once

#include <flame/universe/components/rigid.h>

namespace flame
{
	namespace physics
	{
		struct Rigid;
		struct Shape;
	}

	struct cNodePrivate;
	struct sPhysicsWorldPrivate;

	struct cRigidPrivate : cRigid // R ~ on_*
	{
		bool dynamic = true;
		physics::Rigid* phy_rigid = nullptr;
		std::vector<physics::Shape*> phy_shapes;
		Vec3f staging_impulse = Vec3f(0.f);
		Vec3f curr_coord = Vec3f(0.f);
		Vec4f curr_quat = Vec4f(0.f, 0.f, 0.f, 1.f);

		cNodePrivate* node = nullptr; // R ref
		sPhysicsWorldPrivate* physics_world = nullptr; // R ref

		bool get_dynamic() const override { return dynamic; }
		void set_dynamic(bool v) override;

		void add_impulse(const Vec3f& v) override;

		void on_gain_node();
		void on_lost_node();
		void on_gain_physics_world();
		void on_lost_physics_world();

		void set_pose();
		void get_pose();
	};
}
