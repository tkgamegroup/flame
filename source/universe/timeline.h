#pragma once

#include "universe.h"

namespace flame
{
	// Reflect
	struct Keyframe
	{
		float time;
		std::string value;

		Keyframe() {}

		Keyframe(float t, const std::string& v) :
			time(t),
			value(v)
		{
		}
	};

	// Reflect
	struct Track
	{
		std::string address;
		std::vector<Keyframe> keyframes;
	};

	struct Timeline
	{
		std::vector<Track> tracks;

		std::filesystem::path filename;

		virtual ~Timeline() {}

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
