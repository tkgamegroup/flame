#pragma once

#include "timeline.h"

namespace flame
{
	struct TimelinePrivate : Timeline
	{
		void save(const std::filesystem::path& filename) override;
	};

	struct TimelineInstancePrivate : TimelineInstance
	{
		struct Keyframe
		{
			float time;
			float value;
		};

		struct Track
		{
			float start_time;
			float duration;
			const Attribute* attr;
			void* obj;
			uint component_index;
			std::vector<Keyframe> keyframes;
			uint current_keyframe = 0;

			void update(float t);
		};

		void* ev = nullptr;
		float time = 0.f;
		std::list<Track> tracks;

		TimelineInstancePrivate(TimelinePtr tl, EntityPtr e);

		void play() override;
		void stop() override;
		bool update();
	};
}
