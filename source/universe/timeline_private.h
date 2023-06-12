#pragma once

#include "timeline.h"

namespace flame
{
	struct Action
	{
		float start_time;
		float duration;

		virtual void start(EntityPtr e) = 0;
		virtual void update(float t) = 0;
	};

	struct MoveToAction : Action
	{
		std::string name;
		EntityPtr target;
		vec2 disp;

		void start(EntityPtr e) override;
		void update(float t) override;
	};

	struct CallbackAction : Action
	{
		std::function<void()> cb;

		void start(EntityPtr e) override;
		void update(float t) override;
	};

	struct TimelinePrivate : Timeline
	{
		std::vector<std::unique_ptr<Action>> actions;

		void* start_play(EntityPtr e, float speed) override;

		void insert_action(Action* action, float delay);
		TimelinePtr move_to(const std::string& name, const vec2& disp, float duration, float delay) override;
		void add_callback(const std::function<void()>& cb, float delay) override;

		void save(const std::filesystem::path& filename) override;
	};

	struct ExecutingTimeline
	{
		float time = 0.f;
		bool paused = false;
		std::list<Action*> actions;

		bool update();
	};
}
