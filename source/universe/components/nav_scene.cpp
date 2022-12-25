#include "node_private.h"
#include "nav_scene_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	void cNavScenePrivate::start()
	{
		_frame = generate_delay_frame;
	}

	void cNavScenePrivate::update()
	{
		if (enable)
		{
			if (_frame > 0)
				_frame--;
			else
			{
				sScene::instance()->generate_navmesh(agent_radius, agent_height, walkable_climb, walkable_slope_angle);
				add_event([this]() {
					for (auto cb : finished_callback.list)
						cb.first();
					return false;
				});
				_frame = -1;
			}
		}
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
