#pragma once

#include "nav_agent.h"

namespace flame
{
	struct cNavAgentPrivate : cNavAgent
	{
		int dt_id = -1;
		vec3 prev_pos;

		void set_target(const vec3& pos) override;
		void stop() override;
		vec3 desire_velocity() override;
		vec3 current_velocity() override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};
}
