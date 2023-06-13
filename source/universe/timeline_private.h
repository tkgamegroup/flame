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

	struct ExecutingStrip 
	{
		float start_time;
		float duration;
		const Attribute* attr;
		void* obj;
		uint index;
		float value0;
		float value1;

		std::function<void()> finished_action;

		void update(float t);
	};

	struct ExecutingTimeline
	{
		float time = 0.f;
		bool paused = false;
		std::list<ExecutingStrip> strips;

		ExecutingTimeline(TimelinePtr tl, EntityPtr e);
		bool update();
	};
}
