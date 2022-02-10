#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "../octree.h"
#include "scene_private.h"

namespace flame
{
	sScenePrivate::sScenePrivate()
	{
		octree = new OctNode(999999999.f, vec3(0.f));
	}

	void sScenePrivate::update_transform(EntityPtr e, bool mark_dirty)
	{
		if (!e->global_enable)
			return;

		auto is_static = false;
		if (auto node = e->get_component_i<cNodeT>(0); node)
		{
			is_static = (int)node->is_static == 2;
			if (!is_static)
			{
				if (node->is_static)
					node->is_static = 2;
				if (mark_dirty)
					node->mark_transform_dirty();
				if (node->update_transform())
				{
					if (!node->measurers.list.empty())
					{
						node->bounds.reset();
						for (auto m : node->measurers.list)
						{
							AABB b;
							if (m.first(&b))
								node->bounds.expand(b);
						}
					}
					else if (!node->drawers.list.empty())
						node->bounds = AABB(AABB(vec3(0.f), 10000.f).get_points(node->transform));
					if (node->bounds.invalid())
					{
						if (node->octnode)
							node->octnode->remove(node);
					}
					else
					{
						if (node->octnode)
							node->octnode->add(node);
						else
							octree->add(node);
					}

					mark_dirty = true;
				}
			}
		}

		if (!is_static)
		{
			for (auto& c : e->children)
				update_transform(c.get(), mark_dirty);
		}
	}

	void sScenePrivate::update()
	{
		update_transform(world->root.get(), false);
	}

	static sScenePtr _instance = nullptr;

	struct sSceneInstance : sScene::Instance
	{
		sScenePtr operator()() override
		{
			return _instance;
		}
	}sScene_instance_private;
	sScene::Instance& sScene::instance = sScene_instance_private;

	struct sSceneCreate : sScene::Create
	{
		sScenePtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sScenePrivate();
			return _instance;
		}
	}sScene_create_private;
	sScene::Create& sScene::create = sScene_create_private;
}
