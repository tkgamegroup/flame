#include "../xml.h"
#include "universe_private.h"
#include "timeline_private.h"
#include "entity_private.h"
#include "components/element_private.h"

namespace flame
{
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

	void TimelineInstancePrivate::Track::update(float t)
	{
		if (attr)
		{
			if (t > keyframes[current_keyframe].time)
			{
				current_keyframe++;
				if (current_keyframe >= keyframes.size())
					current_keyframe = 0;
			}

			auto pdata = attr->get_value(obj, true);
			auto ti = (TypeInfo_Data*)attr->type;
			switch (ti->data_type)
			{
			case DataFloat:
			{
				float v;
				if (keyframes.size() == 1)
					v = keyframes[current_keyframe].value;
				else
				{
					auto& k0 = keyframes[current_keyframe];
					auto& k1 = keyframes[current_keyframe + 1];
					v = mix(k0.value, k1.value, (t - k0.time) / (k1.time - k0.time));
				}
				((float*)pdata)[component_index] = v;
			}
				break;
			}
			attr->set_value(obj, pdata);
		}
	}

	void TimelineInstancePrivate::play()
	{
		if (playing)
			return;
		playing = true;

		ev = add_event([this]() {
			return update();
		});
	}

	void TimelineInstancePrivate::stop()
	{
		if (!playing)
			return;
		playing = false;

		remove_event(ev);
	}

	TimelineInstancePrivate::TimelineInstancePrivate(TimelinePtr tl, EntityPtr e)
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
			resolve_address(t.address, e, et.attr, et.obj, et.component_index);
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
							ek.value = s2t<float>(k.value);
							if (k.incremental)
							{
								auto pdata = et.attr->get_value(et.obj, true);
								ek.value += ((float*)pdata)[et.component_index];
							}
							break;
						}
					}
				}
			}
		}
	}

	bool TimelineInstancePrivate::update()
	{
		if (tracks.empty())
		{
			playing = false;
			return false;
		}
		for (auto it = tracks.begin(); it != tracks.end();)
		{
			auto& t = *it;
			if (time < t.start_time)
				break;
			if (time < t.start_time + t.duration)
			{
				t.update(time);
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

	struct TimelineInstanceCreate : TimelineInstance::Create
	{
		TimelineInstancePtr operator()(TimelinePtr timeline, EntityPtr e) override
		{
			return new TimelineInstancePrivate(timeline, e);
		}
	}TimelineInstance_create;
	TimelineInstance::Create& TimelineInstance::create = TimelineInstance_create;
}
