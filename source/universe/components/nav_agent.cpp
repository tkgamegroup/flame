#include "node_private.h"
#include "nav_agent_private.h"
#include "../systems/scene_private.h"

#ifdef USE_RECASTNAV
#include <DetourCrowd.h>
#endif

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
			dt_crowd->requestMoveTarget(dt_id, poly_ref, &pos[0]);
			//printf("%s -> %s\n", str(node->g_pos).c_str(), str(pos).c_str());
		}
#endif
	}

	void cNavAgentPrivate::stop()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto dt_crowd = sScene::instance()->dt_crowd;
			dt_crowd->resetMoveTarget(dt_id);
		}
#endif
	}

	vec3 cNavAgentPrivate::desire_velocity()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto dt_crowd = sScene::instance()->dt_crowd;
			auto agent = dt_crowd->getAgent(dt_id);
			return *(vec3*)agent->dvel;
		}
#endif
		return vec3(0.f);
	}

	vec3 cNavAgentPrivate::current_velocity()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			auto dt_crowd = sScene::instance()->dt_crowd;
			auto agent = dt_crowd->getAgent(dt_id);
			return *(vec3*)agent->vel;
		}
#endif
		return vec3(0.f);
	}

	void cNavAgentPrivate::on_active()
	{
#ifdef USE_RECASTNAV
		auto scene = sScene::instance();
		for (auto ag : scene->peeding_nav_agents)
		{
			if (ag == this)
				return;
		}
		scene->peeding_nav_agents.push_back(this);
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
		else
		{
			std::erase_if(sScene::instance()->peeding_nav_agents, [&](const auto& ag) {
				return ag == this;
			});
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
			if (length(dir) > 0.f)
			{
				auto ang0 = node->get_eul().x;
				auto ang1 = mod(degrees(atan2(dir.x, dir.z)), 360.f); if (ang1 < 0.f) ang1 += 360.f;
				auto dist1 = ang0 - ang1; if (dist1 < 0.f) dist1 += 360.f;
				auto dist2 = ang1 - ang0; if (dist2 < 0.f) dist2 += 360.f;
				//printf("ang0: %f, ang1: %f, dist1: %f, dist2: %f\n", ang0, ang1, dist1, dist2);
				auto tsp = turn_speed * delta_time;
				if (dist1 < dist2)
					node->add_eul(vec3(-min(tsp, dist1), 0.f, 0.f));
				else
					node->add_eul(vec3(+min(tsp, dist2), 0.f, 0.f));
				*(vec3*)agent->npos -= *(vec3*)agent->disp;
				if (min(dist1, dist2) < 15.f)
				{
					prev_pos = *(vec3*)agent->npos;
					node->set_pos(prev_pos);
				}
				else
				{
					*(vec3*)agent->npos = prev_pos;
					*(vec3*)agent->vel = vec3(0.f);
				}
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
