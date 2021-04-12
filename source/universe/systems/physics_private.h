#pragma once

#include "physics.h"

namespace flame
{
	struct sPhysicsPrivate : sPhysics
	{
		std::vector<cRigidPrivate*> rigids;
		std::vector<cControllerPrivate*> controllers;
		cElementPrivate* visualization_layer = nullptr;

		physics::Scene* phy_scene = nullptr;

		vec3 raycast(const vec3& origin, const vec3& dir) override;
		void set_visualization(bool v) override;

		void on_added() override;
		void update() override;
	};
}
