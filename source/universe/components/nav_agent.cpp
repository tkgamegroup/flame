#include "node_private.h"
#include "nav_agent_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	std::vector<cNavAgentPtr> nav_agents;

	void cNavAgentPrivate::set_target(const vec3& pos, bool _face_mode)
	{
		if (target_pos == pos && face_mode == _face_mode)
			return;
		if (face_mode != _face_mode && !face_mode)
			stop();
		target_pos = pos;
		face_mode = _face_mode;
		if (!face_mode)
		{
#ifdef USE_RECASTNAV
			if (dt_id != -1)
			{
				auto scene = sScene::instance();
				if (dt_crowd)
				{
					dtPolyRef poly_ref = dt_nearest_poly(pos);
					dt_crowd->requestMoveTarget(dt_id, poly_ref, &pos[0]);
					//printf("%s -> %s\n", str(node->g_pos).c_str(), str(pos).c_str());
					auto agent = dt_crowd->getEditableAgent(dt_id);
					*(vec3*)agent->dvel = node->rot[2];
				}
			}
#endif
		}
	}

	void cNavAgentPrivate::stop()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			if (dt_crowd)
				dt_crowd->resetMoveTarget(dt_id);
		}
#endif
	}

	vec3 cNavAgentPrivate::desire_velocity()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			if (dt_crowd)
			{
				auto agent = dt_crowd->getAgent(dt_id);
				return *(vec3*)agent->dvel;
			}
		}
#endif
		return vec3(0.f);
	}

	vec3 cNavAgentPrivate::current_velocity()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			if (dt_crowd)
			{
				auto agent = dt_crowd->getAgent(dt_id);
				return *(vec3*)agent->vel;
			}
		}
#endif
		return vec3(0.f);
	}

	void cNavAgentPrivate::on_active()
	{
		nav_agents.push_back(this);
	}

	void cNavAgentPrivate::on_inactive()
	{
		std::erase_if(nav_agents, [&](const auto& ag) {
			return ag == this;
		});
#ifdef USE_RECASTNAV
		if (dt_id != -1)
		{
			if (dt_crowd)
				dt_crowd->removeAgent(dt_id);
			dt_id = -1;
		}
#endif
	}

	void cNavAgentPrivate::update()
	{
		if (face_mode)
		{
			auto dir = target_pos - node->g_pos;
			dir = normalize(dir);
			auto diff = angle_diff(node->get_eul().x, degrees(atan2(dir.x, dir.z)));
			node->add_eul(vec3(sign_min(diff, turn_speed * delta_time), 0.f, 0.f));
		}
		else
		{
#ifdef USE_RECASTNAV
			if (dt_id != -1)
			{
				if (dt_crowd)
				{
					auto agent = dt_crowd->getEditableAgent(dt_id);
					auto dir = *(vec3*)agent->dvel;
					if (length(dir) > 0.f)
					{
						auto diff = angle_diff(node->get_eul().x, degrees(atan2(dir.x, dir.z)));
						node->add_eul(vec3(sign_min(diff, turn_speed * delta_time), 0.f, 0.f));
						*(vec3*)agent->npos -= *(vec3*)agent->disp;
						if (abs(diff) < 15.f)
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
			}
#endif
		}
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
