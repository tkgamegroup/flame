#include "node_private.h"
#include "nav_obstacle_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	std::vector<cNavObstaclePtr> nav_obstacles;

	void cNavObstaclePrivate::on_active()
	{
		nav_obstacles.push_back(this);
	}

	void cNavObstaclePrivate::on_inactive()
	{
		std::erase_if(nav_obstacles, [&](const auto& ag) {
			return ag == this;
		});
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			if (dt_tile_cache)
				dt_tile_cache->removeObstacle(dt_id);
			dt_id = -1;
		}
#endif
	}

	struct cNavObstacleCreate : cNavObstacle::Create
	{
		cNavObstaclePtr operator()(EntityPtr e) override
		{
			return new cNavObstaclePrivate();
		}
	}cNavObstacle_create;
	cNavObstacle::Create& cNavObstacle::create = cNavObstacle_create;
}
