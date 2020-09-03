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
		scene->update(looper().get_delta_time());
		for (auto r : rigids)
		{
			r->retrieving = true;
			Vec3f coord;
			Vec4f quat;
			r->rigid->get_pose(coord, quat);
			r->node->set_pos(coord);
			r->node->set_quat(quat);
			r->retrieving = false;
		}
	}

	sPhysicsWorld* sPhysicsWorld::create()
	{
		return new sPhysicsWorldPrivate();
	}
}
