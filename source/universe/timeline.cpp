#include "timeline_private.h"

namespace flame
{
	void* TimelinePrivate::start_play(EntityPtr e, float speed)
	{
		auto et = new ExecutingTimeline;
		add_event([et]() {
			return et->update();
		});
	}

	void TimelinePrivate::insert_action(Action* action, float delay)
	{
		action->start_time = 0.f;
		if (!actions.empty())
		{
			auto& last = actions.back();
			action->start_time = last->start_time + last->duration;
		}
		action->start_time += delay;

		auto it = std::lower_bound(actions.begin(), actions.end(), action, [](const auto& a, const auto& b) {
			return a->start_time < b->start_time;
		});
		actions.emplace(it, action);
	}

	TimelinePtr TimelinePrivate::move_to(const std::string& name, const vec2& disp, float duration, float delay)
	{
		auto a = new MoveToAction();
		a->duration = duration;
		a->name = name;
		a->disp = disp;
		insert_action(a, delay);
	}

	void TimelinePrivate::add_callback(const std::function<void()>& cb, float delay)
	{

	}

	void TimelinePrivate::save(const std::filesystem::path& filename)
	{

	}

	void Timeline::pause(void* et)
	{

	}

	void Timeline::resume(void* et)
	{

	}

	void Timeline::stop(void* et)
	{

	}

	bool ExecutingTimeline::update()
	{
		if (actions.empty())
		{
			delete this;
			return false;
		}
		for (auto it = actions.begin(); it != actions.end();)
		{
			auto& a = *it;
			if (time < a->start_time)
				break;
			if (a->duration == 0.f)
			{
				a->update(0.f);
				it = actions.erase(it);
				continue;
			}
			if (time < a->start_time + a->duration)
			{
				a->update(time - a->start_time);
				++it;
			}
			else
				it = actions.erase(it);
		}
		time += delta_time;
		return true;
	}
}
