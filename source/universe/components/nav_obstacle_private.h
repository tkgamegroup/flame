#pragma once

#include "nav_obstacle.h"

namespace flame
{
	struct cNavObstaclePrivate : cNavObstacle
	{
		int dt_id = -1;

		void on_active() override;
		void on_inactive() override;
	};

	extern std::vector<cNavObstaclePtr> nav_obstacles;
}
