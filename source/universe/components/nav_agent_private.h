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
		vec3 target_pos;
		bool face_mode = false;
		vec3 prev_pos;

		void set_target(const vec3& pos, bool face_mode) override;
		void stop() override;
		vec3 desire_velocity() override;
		vec3 current_velocity() override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};

	extern std::vector<cNavAgentPtr> nav_agents;
}
