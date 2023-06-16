#pragma once

#include "universe.h"

namespace flame
{
	// Reflect
	struct Keyframe
	{
		float time;
		std::string value;
		bool incremental = false;

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

		virtual void save(const std::filesystem::path& filename) = 0;

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

	struct BoundTimeline
	{
		bool playing = false;

		virtual ~BoundTimeline() {}

		virtual void play() = 0;
		virtual void stop() = 0;

		struct Create
		{
			virtual BoundTimelinePtr operator()(TimelinePtr timeline, EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
