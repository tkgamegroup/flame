#ifdef USE_PHYSICS_MODULE

#include "../../physics/device.h"
#include "../../physics/rigid.h"
#include "../../physics/shape.h"
#include "../../physics/controller.h"
#include "../../physics/scene.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../components/rigid_private.h"
#include "../components/shape_private.h"
#include "../components/character_controller_private.h"
#include "physics_private.h"
#include "renderer_private.h"

namespace flame
{

	using namespace physics;

	const inline auto gravity = -5.f;

	uint sPhysicsPrivate::Visualizer::draw(uint layer, sRendererPtr s_renderer)
	{
		layer++;
		uint lines_count;
		graphics::Line* lines;
		scene->get_visualization_data(&lines_count, &lines);
		s_renderer->draw_lines(lines_count, lines);
		return layer;
	}

	void sPhysicsPrivate::add_rigid(cRigidPrivate* r)
	{
		rigids.push_back(r);
		physics_scene->add_rigid(r->phy_rigid);
	}

	void sPhysicsPrivate::remove_rigid(cRigidPrivate* r)
	{
		std::erase_if(rigids, [&](const auto& i) {
			return i == r;
		});
		physics_scene->remove_rigid(r->phy_rigid);
	}

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
		{
			if (visualization_layer)
				visualization_layer->remove_drawer(&visualizer);
			visualization_layer = nullptr;
			physics_scene->set_visualization(false);
		}
		else
		{
			if (world && physics_scene)
			{
				auto e_debug = world->root->find_child("debug_layer");
				if (e_debug)
				{
					visualization_layer = e_debug->get_component_i<cElementPrivate>(0);
					if (visualization_layer)
					{
						visualization_layer->add_drawer(&visualizer);
						physics_scene->set_visualization(true);
					}
				}
			}
		}
	}

	void sPhysicsPrivate::on_added()
	{
		physics_scene.reset(physics::Scene::create(nullptr, gravity, 2));
		physics_scene->set_trigger_callback([](Capture& c, TouchType type, Shape* trigger_shape, Shape* other_shape) {
			auto tri_shp = (EntityPtr)trigger_shape->user_data;
			auto oth_shp = (EntityPtr)other_shape->user_data;
			for (auto& l : tri_shp->get_component_t<cShapePrivate>()->rigid->trigger_listeners)
				l->call(type, tri_shp, oth_shp);
		}, Capture().set_thiz(this));
		visualizer.scene = physics_scene.get();
	}

	void sPhysicsPrivate::on_removed()
	{
		physics_scene.reset(nullptr);
	}

	void sPhysicsPrivate::update()
	{
		auto delta_time = get_delta_time();

		for (auto c : controllers)
		{
			if (!c->entity->global_visibility)
				continue;

			auto disp = c->disp;
			disp.y += (c->floating_time + delta_time) * gravity;
			if (c->phy_controller->move(disp, delta_time))
				c->floating_time = 0.f;
			else
				c->floating_time += delta_time;
			c->disp = vec3(0.f);
		}

		physics_scene->update(delta_time);

		for (auto r : rigids)
		{
			if (!r->entity->global_visibility)
				continue;

			if (r->dynamic && !r->is_sleeping() && !r->node->transform_dirty)
			{
				vec3 coord;
				quat qut;
				r->phy_rigid->get_pose(coord, qut);
				r->node->set_pos(coord);
				r->node->set_quat(qut);
			}
		}
		for (auto c : controllers)
		{
			if (!c->entity->global_visibility)
				continue;

			auto coord = c->phy_controller->get_position();
			c->node->set_pos(coord);
		}
	}

	sPhysics* sPhysics::create(void* parms)
	{
		return new sPhysicsPrivate();
	}
}

#endif
