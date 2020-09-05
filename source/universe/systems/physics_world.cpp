#include <flame/physics/scene.h>
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/rigid_private.h"
#include "physics_world_private.h"

namespace flame
{
	void sPhysicsWorldPrivate::on_added()
	{
		scene = (physics::Scene*)world->find_object("flame::physics::Scene");
	}

	void sPhysicsWorldPrivate::update()
	{
		for (auto r : rigids)
			r->set_pose();
		scene->update(looper().get_delta_time());
		for (auto r : rigids)
			r->get_pose();
	}

	sPhysicsWorld* sPhysicsWorld::create()
	{
		return new sPhysicsWorldPrivate();
	}
}
