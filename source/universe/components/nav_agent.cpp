#include "node_private.h"
#include "nav_agent_private.h"
#include "../systems/scene_private.h"

#ifdef USE_RECASTNAV
#include <DetourCrowd.h>
#endif

const auto MinSpeed = 0.01f;

namespace flame
{
	void cNavAgentPrivate::set_target(const vec3& pos)
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto scene = sScene::instance();
			dtPolyRef poly_ref = scene->nav_mesh_nearest_poly(pos);
			auto dt_crowd = scene->dt_crowd;
			auto agent = dt_crowd->getEditableAgent(dt_id);
			agent->params.maxSpeed = MinSpeed;
			dt_crowd->requestMoveTarget(dt_id, poly_ref, &pos[0]);
		}
#endif
	}

	void cNavAgentPrivate::on_active()
	{
#ifdef USE_RECASTNAV
		auto dt_crowd = sScene::instance()->dt_crowd;
		dtCrowdAgentParams parms;
		memset(&parms, 0, sizeof(dtCrowdAgentParams));
		parms.radius = radius;
		parms.height = height;
		parms.maxAcceleration = 8.f;
		parms.maxSpeed = MinSpeed;
		parms.collisionQueryRange = parms.radius * 12.0f;
		parms.pathOptimizationRange = parms.radius * 30.0f;
		parms.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO | 
			DT_CROWD_OBSTACLE_AVOIDANCE;
		parms.userData = this;
		dt_id = dt_crowd->addAgent(&node->g_pos[0], &parms);
		if (dt_id == -1)
		{
			printf("dt crowd add agent failed: -1 is returned\n");
			return;
		}
#endif
	}

	void cNavAgentPrivate::on_inactive()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto dt_crowd = sScene::instance()->dt_crowd;
			dt_crowd->removeAgent(dt_id);
			dt_id = -1;
		}
#endif
	}

	void cNavAgentPrivate::update()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto dt_crowd = sScene::instance()->dt_crowd;
			auto agent = dt_crowd->getEditableAgent(dt_id);
			auto dir = *(vec3*)agent->dvel;
			if (length(dir) > MinSpeed * 0.5f)
			{
				auto ang0 = fmodf(node->get_eul().x, 360.f);
				auto ang1 = fmodf(atan2(dir.z, dir.x) - 90.f, 360.f);
				auto dist1 = ang0 - ang1; if (dist1 < 0.f) dist1 += 360.f;
				auto dist2 = ang1 - ang0; if (dist2 < 0.f) dist2 += 360.f;
				auto tsp = turn_speed * delta_time;
				if (dist1 < dist2)
					node->add_eul(vec3(min(tsp, dist1), 0.f, 0.f));
				else
					node->add_eul(vec3(-min(tsp, dist2), 0.f, 0.f));
				if (min(dist1, dist2) < 15.f)
				{
					agent->params.maxSpeed = speed;
					node->set_pos(*(vec3*)agent->npos);
				}
				else
					agent->params.maxSpeed = MinSpeed;
			}
		}
#endif
	}

	struct cNavAgentCreate : cNavAgent::Create
	{
		cNavAgentPtr operator()(EntityPtr) override
		{
			return new cNavAgentPrivate();
		}
	}cNavAgent_create_private;
	cNavAgent::Create& cNavAgent::create = cNavAgent_create_private;
}
