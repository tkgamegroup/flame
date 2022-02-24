#pragma once

#include "nav_agent.h"

namespace flame
{
	struct cNavAgentPrivate : cNavAgent
	{
		int dt_id = -1;

		void set_target(const vec3& pos) override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};
}
