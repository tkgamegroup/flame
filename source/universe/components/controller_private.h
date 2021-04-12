#pragma once

#include "controller.h"

namespace flame
{
	struct cControllerPrivate : cController
	{
		float radius = 0.f;
		float height = 0.f;
		vec3 disp = vec3(0.f);

		physics::Controller* phy_controller = nullptr;

		cNodePrivate* node = nullptr;
		sPhysicsPrivate* physics = nullptr;

		float get_radius() const override { return radius; }
		void set_radius(float r) override;
		float get_height() const override { return height; }
		void set_height(float h) override;
		void move(const vec3& _disp) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
