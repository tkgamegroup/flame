#pragma once

#include "collider.h"

namespace flame
{
	struct cColliderPrivate : cCollider
	{
		cBpInstancePtr bp_comp = nullptr;
		BlueprintInstanceGroup* on_enter_cb = nullptr;
		BlueprintInstanceGroup* on_exit_cb = nullptr;

		void start() override;
		void update() override;
	};
}

