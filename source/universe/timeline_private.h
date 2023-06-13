#pragma once

#include "../foundation/typeinfo.h"
#include "timeline.h"

namespace flame
{
	struct TimelinePrivate : Timeline
	{
		void* start_play(EntityPtr e, float speed) override;

		void save(const std::filesystem::path& filename) override;
	};

	struct ExecutingTimeline
	{
		struct Keyframe
		{
			float time;
			float value;
		};

		struct Strip
		{
			float start_time;
			float duration;
			const Attribute* attr;
			void* obj;
			uint index;
			std::vector<Keyframe> keyframes;

			std::function<void()> finished_action;

			void update(float t);
		};

		float time = 0.f;
		bool paused = false;
		std::list<Strip> strips;

		ExecutingTimeline(TimelinePtr tl, EntityPtr e);
		bool update();
	};
}
