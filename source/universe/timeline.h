#pragma once

#include "universe.h"

namespace flame
{
	struct Timeline
	{
		virtual void* start_play(EntityPtr e, float speed = 1.f) = 0;

		virtual TimelinePtr move_to(const std::string& name, const vec2& disp, float duration, float delay = 0.f) = 0;
		virtual void add_callback(const std::function<void()>& cb, float delay = 0.f) = 0;

		virtual void save(const std::filesystem::path& filename) = 0;

		FLAME_UNIVERSE_API static void pause(void* et);
		FLAME_UNIVERSE_API static void resume(void* et);
		FLAME_UNIVERSE_API static void stop(void* et);

		struct Create
		{
			virtual TimelinePtr operator()() = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;

		struct Load
		{
			virtual TimelinePtr operator()(const std::filesystem::path& filename) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Load& create;
	};
}
