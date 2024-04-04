#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cBpInstance : Component
	{
		// Reflect
		std::filesystem::path bp_name;
		// Reflect
		virtual void set_bp_name(const std::filesystem::path& bp_name) = 0;

		// Reflect
		virtual void start_coroutine(BlueprintInstanceGroup* group, float delay = 0.f) = 0;

		BlueprintPtr bp = nullptr;
		BlueprintInstancePtr bp_ins = nullptr;

		BlueprintInstanceGroupPtr g_update = nullptr;
		BlueprintInstanceGroupPtr g_on_gui = nullptr;

		Listeners<void(uint)> callbacks;

		virtual void call(uint name) = 0;

		struct Create
		{
			virtual cBpInstancePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
