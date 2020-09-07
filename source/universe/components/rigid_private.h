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

		std::vector<std::unique_ptr<Closure<void(Capture&, physics::TouchType, cShape*, cShape*)>>> trigger_listeners;

		std::vector<uint> trigger_listeners_s;

		cNodePrivate* node = nullptr; // R ref
		sPhysicsWorldPrivate* physics_world = nullptr; // R ref

		~cRigidPrivate();

		bool get_dynamic() const override { return dynamic; }
		void set_dynamic(bool v) override;

		void add_impulse(const Vec3f& v) override;

		void* add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShape* trigger_shape, cShape* other_shape), const Capture& capture) override;
		void remove_trigger_listener(void* lis) override;

		void add_trigger_listener_s(uint slot) override;
		void remove_trigger_listener_s(uint slot) override;

		void on_trigger_event(physics::TouchType type, cShape* trigger_shape, cShape* other_shape) override;

		void on_gain_node();
		void on_lost_node();
		void on_gain_physics_world();
		void on_lost_physics_world();

		void set_pose();
		void get_pose();
	};
}
