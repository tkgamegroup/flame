#pragma once

#include <flame/universe/systems/physics_world.h>

namespace flame
{
	namespace physics
	{
		struct Scene;
	}
	
	struct cRigidPrivate;
	struct cControllerPrivate;

	struct sPhysicsWorldPrivate : sPhysicsWorld
	{
		std::vector<cRigidPrivate*> rigids;
		std::vector<cControllerPrivate*> controllers;

		physics::Scene* phy_scene = nullptr;

		void on_added() override;
		void update() override;
	};
}
