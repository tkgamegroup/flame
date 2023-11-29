#pragma once

#include "../../foundation/blueprint.h"
#include "../universe_private.h"
#include "bp_instance.h"

namespace flame
{
	struct cBpInstancePrivate : cBpInstance
	{
		BlueprintInstanceGroup* update_group = nullptr;
		std::vector<BlueprintInstanceGroup*> co_routines;

		~cBpInstancePrivate();

		void set_bp_name(const std::filesystem::path& bp_name) override;
		void start_coroutine(BlueprintInstanceGroup* group, float delay = 0.f) override;
		void start() override;
		void update() override;
	};
}
