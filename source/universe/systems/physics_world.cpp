#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include <flame/script/script.h>
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/rigid_private.h"
#include "../components/shape_private.h"
#include "physics_world_private.h"

namespace flame
{
	using namespace physics;

	void sPhysicsWorldPrivate::on_added()
	{
		scene = (physics::Scene*)world->find_object("flame::physics::Scene");
		scene->set_trigger_callback([](Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape) {
			auto tri_shp = (cShapePrivate*)trigger_shape->user_data;
			auto oth_shp = (cShapePrivate*)other_shape->user_data;
			tri_shp->rigid->on_trigger_event(type, tri_shp, oth_shp);
		}, Capture().set_thiz(this));
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
