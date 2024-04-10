#include "../entity_private.h"
#include "node_private.h"
#include "bp_instance_private.h"
#include "nav_agent_private.h"
#include "nav_obstacle_private.h"
#include "collider_private.h"
#include "../octree.h"
#include "../systems/scene_private.h"

namespace flame
{
	static float get_radius(EntityPtr e)
	{
		if (auto ag = e->get_component<cNavAgent>(); ag)
			return ag->radius;
		if (auto ob = e->get_component<cNavObstacle>(); ob)
		{
			if (ob->type == cNavObstacle::TypeCylinder)
				return ob->extent.x;
			else
				return max(ob->extent.x, ob->extent.z);
		}
		if (auto bp_comp = e->get_component<cBpInstance>(); bp_comp && bp_comp->bp_ins)
		{
			auto ins = bp_comp->bp_ins;
			return ins->get_variable_as<float>("radius"_h);
		}
		return 0.f;
	}

	void cColliderPrivate::start()
	{
		if (auto bp_comp = entity->get_component<cBpInstanceT>(); bp_comp && bp_comp->bp_ins)
		{
			bp_ins = bp_comp->bp_ins;
			on_enter_cb = bp_ins->find_group("collider_on_enter"_h);
			on_exit_cb = bp_ins->find_group("collider_on_exit"_h);
		}

		radius = get_radius(entity) + radius_expand;
	}

	void cColliderPrivate::update()
	{
		if (radius > 0.f)
		{
			auto pos = node->global_pos();
			std::vector<std::pair<EntityPtr, cNodePtr>> res;
			sScene::instance()->octree->get_colliding(pos, radius, res, any_filter, all_filter, parent_search_times);
			for (auto& i : res)
			{
				auto r = get_radius(i.first);
				if (distance(i.second->global_pos(), pos) < r + radius)
				{
					if (on_enter_cb)
					{
						voidptr inputs[1];
						inputs[0] = &i.first;
						bp_ins->call(on_enter_cb, inputs, nullptr);
					}
				}
			}
		}
	}

	struct cColliderCreate : cCollider::Create
	{
		cColliderPtr operator()(EntityPtr e) override
		{
			return new cColliderPrivate;
		}
	}cCollider_create;
	cCollider::Create& cCollider::create = cCollider_create;
}
