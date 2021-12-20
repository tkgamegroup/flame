#pragma once
#ifdef USE_PHYSICS_MODULE

#include "character_controller.h"

namespace flame
{
	struct cCharacterControllerPrivate : cCharacterController
	{
		float radius = 0.f;
		float height = 0.f;
		float static_friction = 0.2f;
		float dynamic_friction = 0.2f;
		float restitution = 0.3f;
		vec3 disp = vec3(0.f);
		float floating_time = 0.f;

		physics::Controller* phy_controller = nullptr;

		cNodePrivate* node = nullptr;
		sPhysicsPrivate* phy_scene = nullptr;

		float get_radius() const override { return radius; }
		void set_radius(float r) override;
		float get_height() const override { return height; }
		void set_height(float h) override;

		float get_static_friction() const override { return static_friction; }
		void set_static_friction(float v) override;

		float get_dynamic_friction() const override { return dynamic_friction; }
		void set_dynamic_friction(float v) override;

		float get_restitution() const override { return restitution; }
		void set_restitution(float v) override;

		void move(const vec3& _disp) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}

#endif
