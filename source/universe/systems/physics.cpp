#include "../../physics/device.h"
#include "../../physics/rigid.h"
#include "../../physics/shape.h"
#include "../../physics/controller.h"
#include "../../physics/scene.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/element_private.h"
#include "../components/rigid_private.h"
#include "../components/shape_private.h"
#include "../components/controller_private.h"
#include "physics_private.h"

namespace flame
{
	using namespace physics;

	vec3 sPhysicsPrivate::raycast(const vec3& origin, const vec3& dir, EntityPtr* out_e)
	{
		void* p = nullptr;
		auto pos = physics_scene->raycast(origin, dir, 1000.f, &p);
		if (out_e)
			*out_e = (EntityPtr)p;
		return pos;
	}

	void sPhysicsPrivate::set_visualization(bool v)
	{
		if (!v)
			visualization_layer = nullptr;
		else
		{
			if (world && physics_scene)
			{
				visualization_layer = world->root->find_child("debug")->get_component_i<cElementPrivate>(0);
				if (visualization_layer)
				{
					physics_scene->set_visualization(true);
					visualization_layer->add_drawer([](Capture& c, uint layer, sRendererPtr s_renderer) {
						auto thiz = c.thiz<sPhysicsPrivate>();
						if (thiz->visualization_layer)
						{
							layer++;
							uint lines_count;
							Line* lines;
							thiz->physics_scene->get_visualization_data(&lines_count, &lines);
							// TODO: fix below
							//canvas->draw_lines(lines_count, lines);
						}
						return layer;
					}, Capture().set_thiz(this));
				}
			}
		}
	}

	void sPhysicsPrivate::on_added()
	{
		physics_scene.reset(physics::Scene::create(physics::Device::get_default(), -9.81f, 2));
		physics_scene->set_trigger_callback([](Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape) {
			auto tri_shp = (EntityPtr)trigger_shape->user_data;
			auto oth_shp = (EntityPtr)other_shape->user_data;
			for (auto& l : tri_shp->get_component_t<cShapePrivate>()->rigid->trigger_listeners)
				l->call(type, tri_shp, oth_shp);
		}, Capture().set_thiz(this));
	}

	void sPhysicsPrivate::on_removed()
	{
		physics_scene.reset(nullptr);
	}

	void sPhysicsPrivate::update()
	{
		auto delta_time = looper().get_delta_time();

		for (auto c : controllers)
		{
			auto disp = c->disp;
			disp.y -= 1.f;
			// physx::PxVec3 disp(c.x, -gravity * o->floatingTime * o->floatingTime, c.z);
			// o->floatingTime += dist;
			c->phy_controller->move(disp, delta_time);
			c->disp = vec3(0.f);
		}

		physics_scene->update(delta_time);

		for (auto r : rigids)
		{
			if (r->dynamic && !r->node->transform_dirty)
			{
				vec3 coord;
				quat qut;
				r->phy_rigid->get_pose(coord, qut);
				auto pn = r->entity->get_parent_component_t<cNodePrivate>();
				if (pn)
				{
					auto q_inv = inverse(pn->g_qut);
					r->node->set_pos(q_inv * (coord - pn->g_pos) / pn->g_scl);
					r->node->set_quat(q_inv * qut);
				}
				else
				{
					r->node->set_pos(coord);
					r->node->set_quat(qut);
				}
			}
		}
		for (auto c : controllers)
		{
			auto coord = c->phy_controller->get_position();
			auto pn = c->entity->get_parent_component_t<cNodePrivate>();
			if (pn)
			{
				auto q_inv = inverse(pn->g_qut);
				c->node->set_pos(q_inv * (coord - pn->g_pos) / pn->g_scl);
			}
			else
				c->node->set_pos(coord);
		}
	}

	sPhysics* sPhysics::create(void* parms)
	{
		return new sPhysicsPrivate();
	}
}
