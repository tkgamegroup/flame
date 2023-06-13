#include "universe_private.h"
#include "timeline_private.h"
#include "entity_private.h"
#include "components/element_private.h"

namespace flame
{
	void* TimelinePrivate::start_play(EntityPtr e, float speed)
	{
		auto et = new ExecutingTimeline(this, e);
		add_event([et]() {
			return et->update();
		});
		return et;
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

	void ExecutingTimeline::Strip::update(float t)
	{
		if (attr)
		{
			auto ti = (TypeInfo_Data*)attr->type;
		}
	}

	ExecutingTimeline::ExecutingTimeline(TimelinePtr tl, EntityPtr e)
	{
		for (auto& s : tl->strips)
		{
			auto it = strips.begin();
			for (; it != strips.end(); it++)
			{
				if (it->start_time > s.start_time)
					break;
			}
			auto& es = *strips.emplace(it);
			es.start_time = s.start_time;
			es.duration = s.duration;
			resolve_address(s.address, e, es.attr, es.obj, es.index);
			if (es.attr)
			{
				if (es.attr->type->tag != TagD)
					es.attr = nullptr;
				else
				{
					auto ti = (TypeInfo_Data*)es.attr->type;
					for (auto& k : s.keyframes)
					{
						auto& ek = es.keyframes.emplace_back();
						ek.time = k.time;
						switch (ti->data_type)
						{
						case DataFloat:
								if (k.value == "-")
								{
									auto v = *(float*)es.attr->get_value(es.obj);
									ek.value = v;
								}
								else
									ek.value = s2t<float>(k.value);
							break;
						}
					}
				}
			}
		}
	}

	bool ExecutingTimeline::update()
	{
		if (strips.empty())
		{
			delete this;
			return false;
		}
		for (auto it = strips.begin(); it != strips.end();)
		{
			auto& m = *it;
			if (time < m.start_time)
				break;
			if (time < m.start_time + m.duration)
			{
				m.update(time - m.start_time);
				++it;
			}
			else
			{
				if (m.finished_action)
					m.finished_action();
				it = strips.erase(it);
			}
		}
		time += delta_time;
		return true;
	}

	struct TimelineCreate : Timeline::Create
	{
		TimelinePtr operator()() override
		{
			return new TimelinePrivate();
		}
	}Timeline_create;
	Timeline::Create& Timeline::create = Timeline_create;

	struct TimelineLoad : Timeline::Load
	{
		TimelinePtr operator()(const std::filesystem::path& filename) override
		{
			auto ret = new TimelinePrivate();
			return ret;
		}
	}Timeline_load;
	Timeline::Load& Timeline::load = Timeline_load;
}
