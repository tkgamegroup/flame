#pragma once

#include "timeline.h"

namespace flame
{
	struct ExecutingAction;

	struct Action
	{
		float start_time;
		float duration;

		virtual ExecutingAction* make_executing(EntityPtr e) = 0;
	};

	struct MoveToAction : Action
	{
		std::string name;
		vec2 disp;

		ExecutingAction* make_executing(EntityPtr e) override;
	};

	struct CallbackAction : Action
	{
		std::function<void()> cb;

		ExecutingAction* make_executing(EntityPtr e) override;
	};

	struct TimelinePrivate : Timeline
	{
		std::vector<std::unique_ptr<Action>> actions;

		void* start_play(EntityPtr e, float speed) override;

		void insert_action(Action* action, float delay);
		TimelinePtr move_to(const std::string& name, const vec2& disp, float duration, float delay) override;
		TimelinePtr add_callback(const std::function<void()>& cb, float delay) override;

		void save(const std::filesystem::path& filename) override;
	};

	struct ExecutingAction
	{
		float start_time;
		float duration;

		virtual void update(float t) = 0;
	};

	struct MoveToExecutingAction : ExecutingAction
	{
		cElementPtr target;
		vec2 p0, p1;

		void update(float t) override;
	};

	struct CallbackExecutingAction : ExecutingAction
	{
		std::function<void()> cb;

		void update(float t) override;
	};

	struct ExecutingTimeline
	{
		float time = 0.f;
		bool paused = false;
		std::list<std::unique_ptr<ExecutingAction>> actions;

		ExecutingTimeline(TimelinePtr tl, EntityPtr e);
		bool update();
	};
}
