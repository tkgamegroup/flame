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
	struct sPhysicsPrivate;

	struct cRigidPrivate : cRigid
	{
		bool dynamic = true;
		physics::Rigid* phy_rigid = nullptr;
		std::vector<physics::Shape*> phy_shapes;
		vec3 staging_impulse = vec3(0.f);

		std::vector<std::unique_ptr<Closure<void(Capture&, physics::TouchType, cShape*, cShape*)>>> trigger_listeners;

		std::vector<uint> trigger_listeners_s;

		cNodePrivate* node = nullptr;
		sPhysicsPrivate* physics = nullptr;

		~cRigidPrivate();

		bool get_dynamic() const override { return dynamic; }
		void set_dynamic(bool v) override;

		void add_impulse(const vec3& v) override;

		void* add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShape* trigger_shape, cShape* other_shape), const Capture& capture) override;
		void remove_trigger_listener(void* lis) override;

		void add_trigger_listener_s(uint slot) override;
		void remove_trigger_listener_s(uint slot) override;

		void on_trigger_event(physics::TouchType type, cShape* trigger_shape, cShape* other_shape) override;

		void create();
		void destroy();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
