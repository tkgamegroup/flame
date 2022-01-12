#include "../entity_private.h"
#include "../world_private.h"
#include "../components/node_private.h"
#include "scene_private.h"

namespace flame
{
	void update_transform(EntityPtr e)
	{
		if (!e->global_enable)
			return;

		if (auto node = e->get_component_i<cNodeT>(0); node)
			node->update_transform();

		for (auto& c : e->children)
			update_transform(c.get());
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
		sScenePtr operator()(WorldPtr) override
		{
			assert(!_instance);
			_instance = new sScenePrivate();
			return _instance;
		}
	}sScene_create_private;
	sScene::Create& sScene::create = sScene_create_private;
}
