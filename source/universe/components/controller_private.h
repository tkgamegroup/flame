#pragma once

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
	
	struct cControllerPrivate : cController // R ~ on_*
	{
		Vec3f disp = Vec3f(0.f);

		physics::Controller* phy_controller = nullptr;

		cNodePrivate* node = nullptr; // R ref
		cMeshPrivate* mesh = nullptr; // R ref
		sPhysicsWorldPrivate* physics_world = nullptr; // R ref

		void move(const Vec3f& _disp) override { disp = _disp; }

		void on_gain_physics_world();
		void on_lost_physics_world();
	};
}
