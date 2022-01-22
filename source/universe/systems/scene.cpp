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

	void sScenePrivate::update_transform(EntityPtr e)
	{
		if (!e->global_enable)
			return;

		auto is_static = false;
		if (auto node = e->get_component_i<cNodeT>(0); node)
		{
			is_static = (int)node->is_static == 2;
			if (!is_static)
			{
				auto dirty = node->transform_dirty;
				node->update_transform();
				if (node->is_static)
					node->is_static = 2;
				if (dirty)
				{
					node->bounds.reset();
					if (!node->measurers.list.empty())
					{
						for (auto m : node->measurers.list)
						{
							AABB b;
							if (m(&b))
								node->bounds.expand(b);
						}
					}
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
				}
			}
		}

		if (!is_static)
		{
			for (auto& c : e->children)
				update_transform(c.get());
		}
	}

	void sScenePrivate::update()
	{
		update_transform(world->root.get());
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
