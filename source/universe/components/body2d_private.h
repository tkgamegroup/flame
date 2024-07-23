#pragma once

#include "body2d.h"
#if USE_PHYSICS_MODULE
#include "../../physics/body2d.h"
#endif

namespace flame
{
	struct cBody2dPrivate : cBody2d
	{
		physics::Body2dPtr body = nullptr;

		~cBody2dPrivate();

		vec2 get_velocity() override;
		void set_velocity(const vec2& vel) override;
		void upload_pos() override;
		void apply_force(const vec2& force) override;

		void on_active() override;
		void on_inactive() override;
		void update() override;
	};

	extern std::vector<cBody2dPtr> bodies2d;
}
