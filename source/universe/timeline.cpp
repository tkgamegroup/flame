#include "timeline_private.h"
#include "entity_private.h"
#include "components/element_private.h"

namespace flame
{
	ExecutingAction* MoveToAction::make_executing(EntityPtr e)
	{
		auto a = new MoveToExecutingAction;
		if (name.empty())
			a->target = e->element();
		else
		{
			e = e->find_child_recursively(name);
			a->target = e ? e->element() : nullptr;
		}
		if (a->target)
		{
			a->p0 = a->target->pos;
			a->p1 = a->p0 + disp;
		}
		return a;
	}

	ExecutingAction* CallbackAction::make_executing(EntityPtr e)
	{
		auto a = new CallbackExecutingAction;
		a->cb = cb;
		return a;
	}

	void* TimelinePrivate::start_play(EntityPtr e, float speed)
	{
		auto et = new ExecutingTimeline(this, e);
		add_event([et]() {
			return et->update();
		});
		return et;
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
		return this;
	}

	TimelinePtr TimelinePrivate::add_callback(const std::function<void()>& cb, float delay)
	{
		auto a = new CallbackAction();
		a->duration = 0.f;
		a->cb = cb;
		insert_action(a, delay);
		return this;
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

	void MoveToExecutingAction::update(float t)
	{
		if (target)
			target->set_pos(mix(p0, p1, t / duration));
	}

	void CallbackExecutingAction::update(float t)
	{
		cb();
	}

	ExecutingTimeline::ExecutingTimeline(TimelinePtr tl, EntityPtr e)
	{

		for (auto& a : tl->actions)
		{
			auto ea = a->make_executing(e);
			ea->start_time = a->start_time;
			ea->duration = a->duration;
			actions.emplace_back(ea);
		}
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
