#pragma once

#include "universe.h"

namespace flame
{
	// Reflect
	struct Strip
	{
		float start_time;
		float duration;
		std::string address;
		std::string value0;
		std::string value1;

		// Reflect
		VirtualUdt<Action>	finished_action;
	};

	struct Timeline
	{
		virtual ~Timeline() {}

		std::vector<Strip> strips;

		virtual void* start_play(EntityPtr e, float speed = 1.f) = 0;

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
		FLAME_UNIVERSE_API static Load& load;
	};
}
