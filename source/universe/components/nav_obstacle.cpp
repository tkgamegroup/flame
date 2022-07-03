#include "node_private.h"
#include "nav_obstacle_private.h"

namespace flame
{
	struct cNavObstacleCreate : cNavObstacle::Create
	{
		cNavObstaclePtr operator()(EntityPtr e) override
		{
			return new cNavObstaclePrivate();
		}
	}cNavObstacle_create;
	cNavObstacle::Create& cNavObstacle::create = cNavObstacle_create;
}
