#pragma once

#include "collider.h"

namespace flame
{
	struct cColliderPrivate : cCollider
	{
		float radius;
		BlueprintInstancePtr bp_ins = nullptr;
		BlueprintInstanceGroup* on_enter_cb = nullptr;
		BlueprintInstanceGroup* on_exit_cb = nullptr;

		void start() override;
		void update() override;
	};
}

