#pragma once

#include <flame/universe/systems/physics_world.h>

namespace flame
{
	namespace physics
	{
		struct Scene;
	}
	
	struct cRigidPrivate;

	struct sPhysicsWorldPrivate : sPhysicsWorld
	{
		std::vector<cRigidPrivate*> rigids;

		physics::Scene* scene = nullptr;

		void on_added() override;
		void update() override;
	};
}
