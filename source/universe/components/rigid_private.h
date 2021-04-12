#pragma once

#include "rigid.h"

namespace flame
{
	struct cRigidPrivate : cRigid
	{
		bool dynamic = true;
		physics::Rigid* phy_rigid = nullptr;
		std::vector<physics::Shape*> phy_shapes;
		vec3 staging_impulse = vec3(0.f);

		std::vector<std::unique_ptr<Closure<void(Capture&, physics::TouchType, cShapePtr, cShapePtr)>>> trigger_listeners;

		cNodePrivate* node = nullptr;
		sPhysicsPrivate* physics = nullptr;

		~cRigidPrivate();

		bool get_dynamic() const override { return dynamic; }
		void set_dynamic(bool v) override;

		void add_impulse(const vec3& v) override;

		void* add_trigger_listener(void (*callback)(Capture& c, physics::TouchType type, cShapePtr trigger_shape, cShapePtr other_shape), const Capture& capture) override;
		void remove_trigger_listener(void* lis) override;

		void create();
		void destroy();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
