#include "node_private.h"
#include "nav_agent_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	std::vector<cNavAgentPtr> nav_agents;

	void cNavAgentPrivate::set_speed_scale(float v)
	{
		if (speed_scale == v)
			return;
		speed_scale = v;

#ifdef USE_RECASTNAV
		if (dt_id != -1 && dt_crowd)
		{
			auto agent = dt_crowd->getEditableAgent(dt_id);
			agent->params.maxSpeed = speed * speed_scale;
		}
#endif
	}

	void cNavAgentPrivate::set_turn_speed_scale(float v)
	{
		if (turn_speed_scale == v)
			return;
		turn_speed_scale = v;
	}

	void cNavAgentPrivate::set_target(const vec3& pos)
	{
		if (target_pos == pos)
			return;

#ifdef USE_RECASTNAV
		if (dt_id != -1 && dt_crowd)
		{
			target_pos = pos;
			reached = false;
			dist_ang_diff(node->pos, target_pos, node->get_eul().x - 90.f, dist, ang_diff);

			dtPolyRef poly_ref = dt_nearest_poly(pos, vec3(2.f, 4.f, 2.f));
			dt_crowd->requestMoveTarget(dt_id, poly_ref, &pos[0]);
			//printf("%s -> %s\n", str(node->pos).c_str(), str(pos).c_str());
			auto agent = dt_crowd->getEditableAgent(dt_id);
			*(vec3*)agent->dvel = node->z_axis();
		}
#endif
	}

	void cNavAgentPrivate::stop()
	{
		if (dist >= 0)
		{
			dist = -1.f;
#ifdef USE_RECASTNAV
			if (dt_id != -1 && dt_crowd)
				dt_crowd->resetMoveTarget(dt_id);
#endif
		}
	}

	void cNavAgentPrivate::update_pos()
	{
#ifdef USE_RECASTNAV
		if (dt_id != -1 && dt_crowd)
		{
			npos = node->pos;
			auto agent = dt_crowd->getEditableAgent(dt_id);
			*(vec3*)agent->npos = node->pos;
		}
#endif
	}

	void cNavAgentPrivate::on_active()
	{
		if (flying)
			return;
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
		if (!enable || dist < 0.f)
			return;

		dist_ang_diff(node->pos, target_pos, angle_xz(node->z_axis()), dist, ang_diff);

		if (flying)
		{
			auto turn_angle = radians(sign_min(ang_diff, turn_speed * turn_speed_scale * delta_time));
			node->mul_qut(angleAxis(turn_angle, vec3(0.f, 1.f, 0.f)));

			if (dist > stop_distance && ang_diff < 15.f)
				node->add_pos(normalize(target_pos - node->pos) * speed * delta_time);
		}
		else
		{
			if (speed_scale == 0.f)
			{
				auto turn_angle = radians(sign_min(ang_diff, turn_speed * turn_speed_scale * delta_time));
				node->mul_qut(angleAxis(turn_angle, vec3(0.f, 1.f, 0.f)));
			}
			else
			{
#ifdef USE_RECASTNAV
				if (dt_id != -1 && dt_crowd)
				{
					auto agent = dt_crowd->getEditableAgent(dt_id);
					auto dvel = *(vec3*)agent->dvel;
					auto dmag = dot(dvel, dvel);
					if (dmag > 0.1f && dist > stop_distance)
					{
						auto path_ang_diff = angle_diff(angle_xz(node->z_axis()), angle_xz(dvel));
						auto turn_angle = radians(sign_min(path_ang_diff, turn_speed * turn_speed_scale * delta_time));
						node->mul_qut(angleAxis(turn_angle, vec3(0.f, 1.f, 0.f)));
						if (abs(path_ang_diff) < 15.f)
						{
							npos = *(vec3*)agent->npos;
							node->set_pos(npos);
						}
						else
						{
							*(vec3*)agent->npos = npos;
							*(vec3*)agent->vel = vec3(0.f);
						}
					}
					else
					{
						if (agent->targetState == DT_CROWDAGENT_TARGET_VALID)
							reached = true;
					}
				}
#endif
			}
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
