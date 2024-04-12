#pragma once

#include "../../foundation/blueprint.h"
#include "../universe_private.h"
#include "bp_instance.h"

namespace flame
{
	struct cBpInstancePrivate : cBpInstance
	{
		std::vector<BlueprintInstanceGroupPtr> coroutines;
		bool executing_coroutines = false;
		std::vector<BlueprintInstanceGroupPtr> peeding_add_coroutines;

		~cBpInstancePrivate();

		void set_bp_name(const std::filesystem::path& bp_name) override;
		void start_coroutine(BlueprintInstanceGroup* group, float delay = 0.f) override;
		void call(uint name) override;
		void on_message(uint msg, void* data1, void* data2) override;
		void start() override;
		void update() override;
	};
}
