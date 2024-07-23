#pragma once

#include "nav_agent.h"

#ifdef USE_RECASTNAV
#include <DetourCrowd.h>
#endif

namespace flame
{
	struct cNavAgentPrivate : cNavAgent
	{
		int dt_id = -1;
		vec3 npos;

		void set_speed_scale(float v) override;
		void set_turn_speed_scale(float v) override;

		void set_target(const vec3& pos) override;
		void stop() override;
		void upload_pos() override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};

	extern std::vector<cNavAgentPtr> nav_agents;
}
