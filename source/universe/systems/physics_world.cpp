#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include <flame/physics/controller.h>
#include <flame/script/script.h>
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/rigid_private.h"
#include "../components/shape_private.h"
#include "../components/controller_private.h"
#include "physics_world_private.h"

namespace flame
{
	using namespace physics;

	void sPhysicsWorldPrivate::on_added()
	{
		phy_scene = (physics::Scene*)world->find_object("flame::physics::Scene");
		phy_scene->set_trigger_callback([](Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape) {
			auto tri_shp = (cShapePrivate*)trigger_shape->user_data;
			auto oth_shp = (cShapePrivate*)other_shape->user_data;
			tri_shp->rigid->on_trigger_event(type, tri_shp, oth_shp);
		}, Capture().set_thiz(this));
	}

	void sPhysicsWorldPrivate::update()
	{
		for (auto c : controllers)
		{
			auto disp = c->disp;
			disp.y() -= 9.8f;
			c->move(disp);
			//physx::PxVec3 disp(c.x, -gravity * o->floatingTime * o->floatingTime, c.z);
			//o->floatingTime += dist;

			//if (o->pxController->move(disp, 0.f, dist, nullptr) & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
			//	o->floatingTime = 0.f;
		}

		phy_scene->update(looper().get_delta_time());

		for (auto r : rigids)
		{
			if (r->dynamic)
			{
				Vec3f coord;
				Vec4f quat;
				r->phy_rigid->get_pose(coord, quat);
				auto pn = r->entity->get_parent_component_t<cNodePrivate>();
				if (pn)
				{
					auto q_inv = pn->global_quat;
					q_inv = Vec4f(-q_inv.x(), -q_inv.y(), -q_inv.z(), q_inv.w());
					r->node->set_pos(quat_mul(q_inv, coord - pn->global_pos) / pn->global_scale);
					r->node->set_quat(quat_mul(q_inv, quat));
				}
				else
				{
					r->node->set_pos(coord);
					r->node->set_quat(quat);
				}
			}
		}
		for (auto c : controllers)
		{
			//			auto p = o->pxController->getPosition();
			//			auto c = glm::vec3(p.x, p.y, p.z) - o->model->controller_position * o->get_scale();
			//			o->set_coord(c);
		}
	}

	sPhysicsWorld* sPhysicsWorld::create()
	{
		return new sPhysicsWorldPrivate();
	}
}
