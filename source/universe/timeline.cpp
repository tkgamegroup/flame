#include "../xml.h"
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

	void TimelinePrivate::save(const std::filesystem::path& _filename)
	{
		pugi::xml_document doc;
		pugi::xml_node doc_root;
		auto filename = Path::get(_filename);

		doc_root = doc.append_child("timeline");
		doc.save_file(filename.c_str());
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

	void ExecutingTimeline::Track::update(float t)
	{
		if (attr)
		{
			auto ti = (TypeInfo_Data*)attr->type;
		}
	}

	ExecutingTimeline::ExecutingTimeline(TimelinePtr tl, EntityPtr e)
	{
		for (auto& t : tl->tracks)
		{
			auto it = tracks.begin();
			for (; it != tracks.end(); it++)
			{
				if (it->start_time > t.start_time)
					break;
			}
			auto& et = *tracks.emplace(it);
			et.start_time = t.start_time;
			et.duration = t.duration;
			resolve_address(t.address, e, et.attr, et.obj, et.index);
			if (et.attr)
			{
				if (et.attr->type->tag != TagD)
					et.attr = nullptr;
				else
				{
					auto ti = (TypeInfo_Data*)et.attr->type;
					for (auto& k : t.keyframes)
					{
						auto& ek = et.keyframes.emplace_back();
						ek.time = k.time;
						switch (ti->data_type)
						{
						case DataFloat:
								if (k.value == "-")
								{
									auto v = *(float*)et.attr->get_value(et.obj);
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
		if (tracks.empty())
		{
			delete this;
			return false;
		}
		for (auto it = tracks.begin(); it != tracks.end();)
		{
			auto& t = *it;
			if (time < t.start_time)
				break;
			if (time < t.start_time + t.duration)
			{
				t.update(time - t.start_time);
				++it;
			}
			else
				it = tracks.erase(it);
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
		TimelinePtr operator()(const std::filesystem::path& _filename) override
		{
			pugi::xml_document doc;
			pugi::xml_node doc_root;

			auto filename = Path::get(_filename);
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"timeline does not exist: %s\n", _filename.c_str());
				return nullptr;
			}
			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("timeline"))
			{
				wprintf(L"timeline is wrong format: %s\n", _filename.c_str());
				return nullptr;
			}

			auto ret = new TimelinePrivate();
			ret->filename = filename;
			return ret;
		}
	}Timeline_load;
	Timeline::Load& Timeline::load = Timeline_load;
}
