#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "scene_private.h"

namespace flame
{
	void update_transform(EntityPtr e, bool dirty)
	{
		if (!e->global_enable)
			return;

		auto node = e->get_component_i<cNodeT>(0);

		if (!node)
			return;

		if (!dirty && !node->transform_dirty)
			return;

		node->update_transform();

		for (auto& c : e->children)
			update_transform(c.get(), true);
	}

	void sScenePrivate::update()
	{
		if (!world->first_node)
			return;
		update_transform(world->first_node, false);
	}

	struct sSceneCreate : sScene::Create
	{
		sScenePtr operator()(WorldPtr) override
		{
			return new sScenePrivate();
		}
	}sScene_create_private;
	sScene::Create& sScene::create = sScene_create_private;
}
