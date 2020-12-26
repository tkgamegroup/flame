#pragma once

#include <flame/universe/systems/physics.h>

namespace flame
{
	namespace physics
	{
		struct Scene;
	}
	
	struct cElementPrivate;
	struct cRigidPrivate;
	struct cControllerPrivate;

	struct sPhysicsPrivate : sPhysics
	{
		std::vector<cRigidPrivate*> rigids;
		std::vector<cControllerPrivate*> controllers;
		cElementPrivate* visualization_layer = nullptr;

		physics::Scene* phy_scene = nullptr;

		void set_visualization(bool v) override;

		void on_added() override;
		void update() override;
	};
}
