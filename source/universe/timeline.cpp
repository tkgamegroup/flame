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

		auto n_tracks = doc_root.append_child("tracks");
		for (auto& t : tracks)
		{
			auto n_track = n_tracks.append_child("track");
			n_track.append_attribute("address").set_value(t.address.c_str());
			auto n_keyframes = n_track.append_child("keyframes");
			for (auto& kf : t.keyframes)
			{
				auto n_keyframe = n_track.append_child("keyframe");
				n_keyframe.append_attribute("time").set_value(kf.time);
				n_keyframe.append_attribute("value").set_value(kf.value.c_str());
			}
		}

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
			if (t.keyframes.empty())
				continue;
			auto start_time = t.keyframes.front().time;
			auto duration = t.keyframes.back().time - start_time;

			auto it = std::lower_bound(tracks.begin(), tracks.end(), start_time, [](const auto& a, auto t) { 
				return a.start_time < t; 
			});
			auto& et = *tracks.emplace(it);
			et.start_time = start_time;
			et.duration = duration;
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

			for (auto n_track : doc_root.child("tracks"))
			{
				auto& t = ret->tracks.emplace_back();
				t.address = n_track.attribute("address").as_string();
				for (auto n_keyframe : n_track.child("keyframes"))
				{
					auto& kf = t.keyframes.emplace_back();
					kf.time = n_keyframe.attribute("time").as_float();
					kf.value = n_keyframe.attribute("value").as_string();
				}
			}

			ret->filename = filename;
			return ret;
		}
	}Timeline_load;
	Timeline::Load& Timeline::load = Timeline_load;
}
