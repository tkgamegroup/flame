#include "../entity_private.h"
#include "node_private.h"
#include "bp_instance_private.h"
#include "collider_private.h"
#include "../octree.h"
#include "../systems/scene_private.h"

namespace flame
{
	void cColliderPrivate::start()
	{
		bp_comp = entity->get_component<cBpInstanceT>();
		if (bp_comp && bp_comp->bp_ins)
		{
			auto ins = bp_comp->bp_ins;
			if (radius == -1.f)
				radius = ins->get_variable<float>("radius"_h) + radius_add;
			on_enter_cb = ins->find_group("on_enter"_h);
			on_exit_cb = ins->find_group("on_exit"_h);
		}
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
				auto r = 0.f;
				if (auto cbp = i.first->get_component<cBpInstanceT>(); cbp && cbp->bp_ins)
					r = cbp->bp_ins->get_variable<float>("radius"_h);
				if (distance(i.second->global_pos(), pos) < r + radius)
				{
					if (on_enter_cb)
					{
						voidptr inputs[1];
						inputs[0] = &i.first;
						bp_comp->bp_ins->call(on_enter_cb, inputs, nullptr);
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
