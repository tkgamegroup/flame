#include "node_private.h"
#include "nav_scene_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	void cNavScenePrivate::start()
	{
		sScene::instance()->generate_nav_mesh(agent_radius, agent_height, walkable_climb, walkable_slope_angle);
	}

	struct cNavSceneCreate : cNavScene::Create
	{
		cNavScenePtr operator()(EntityPtr e) override
		{
			return new cNavScenePrivate();
		}
	}cNavScene_create;
	cNavScene::Create& cNavScene::create = cNavScene_create;
}
