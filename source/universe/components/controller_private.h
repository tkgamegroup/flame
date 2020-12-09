#pragma once

#include "../entity_private.h"
#include <flame/universe/components/controller.h>

namespace flame
{
	namespace physics
	{
		struct Controller;
	}

	struct cNodePrivate;
	struct cMeshPrivate;
	struct sPhysicsWorldPrivate;
	
	struct cControllerPrivate : cController
	{
		float radius = 0.f;
		float height = 0.f;
		vec3 disp = vec3(0.f);

		physics::Controller* phy_controller = nullptr;

		cNodePrivate* node = nullptr;
		sPhysicsWorldPrivate* physics_world = nullptr;

		float get_radius() const override { return radius; }
		void set_radius(float r) override;
		float get_height() const override { return height; }
		void set_height(float h) override;
		void move(const vec3& _disp) override { disp = _disp; }

		void on_gain_physics_world();
		void on_lost_physics_world();
	};
}
