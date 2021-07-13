#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "armature_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cArmaturePrivate::Action::apply(Bone* bones, uint frame)
	{
		for (auto& t : tracks)
		{
			auto& b = bones[t.first];
			auto& k = t.second[frame];
			b.node->set_pos(k.p);
			b.node->set_quat(k.q);
		}
	}

	cArmaturePrivate::~cArmaturePrivate()
	{
		stop();
	}

	void cArmaturePrivate::set_model(const std::filesystem::path& name)
	{
		if (model_name == name)
			return;
		bones.clear();
		model_name = name;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cArmaturePrivate::set_animations(const std::wstring& _animation_names)
	{
		if (animation_names == _animation_names)
			return;
		animation_names = _animation_names;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"src"_h>);
	}

	void cArmaturePrivate::play(uint id)
	{
		if (playing == id)
			return;
		playing = id;
		frame = 0;
		frame_accumulate = 0.f;
		if (!event)
		{
			event = looper().add_event([](Capture& c) {
				auto thiz = c.thiz<cArmaturePrivate>();
				thiz->advance();
				if (thiz->frame != -1)
					c._current = nullptr;
				else
				{
					thiz->event = nullptr;
					thiz->stop();
				}
			}, Capture().set_thiz(this), 1U);
		}
	}

	void cArmaturePrivate::stop()
	{
		playing = -1;
		if (event)
		{
			looper().remove_event(event);
			event = nullptr;
		}
	}

	void cArmaturePrivate::stop_at(uint id, int frame)
	{
		stop();

		auto& a = actions[id];
		peeding_pose = { id, frame < 0 ? a.total_frame + frame : frame };
	}

	void cArmaturePrivate::set_loop(bool l)
	{
		if (loop == l)
			return;
		loop = l;
	}

	void* cArmaturePrivate::add_callback(void (*callback)(Capture& c, int frame), const Capture& capture)
	{
		if (!callback)
		{
			auto slot = (uint)&capture;
			callback = [](Capture& c, int f) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<int>());
				scr_ins->get_member("f");
				scr_ins->push_int(f);
				scr_ins->call(1);
				scr_ins->pop(2);
			};
			auto c = new Closure(callback, Capture().set_data(&slot));
			callbacks.emplace_back(c);
			return c;
		}
		auto c = new Closure(callback, capture);
		callbacks.emplace_back(c);
		return c;
	}

	void cArmaturePrivate::remove_callback(void* cb)
	{
		std::erase_if(callbacks, [&](const auto& i) {
			return i == (decltype(i))cb;
		});
	}

	void cArmaturePrivate::apply_src()
	{
		if (bones.empty() && entity)
		{
			graphics::Model* model = nullptr;
			auto fn = model_name;
			if (fn.extension().empty())
				model = graphics::Model::get_standard(fn.c_str());
			else
			{
				if (!fn.is_absolute())
					fn = entity->get_src(src_id).parent_path() / fn;
				fn.make_preferred();
				model = graphics::Model::get(fn.c_str());
			}
			fassert(model);

			auto bones_count = model->get_bones_count();
			fassert(bones_count);

			bones.resize(bones_count);
			bone_mats.resize(bones_count);
			for (auto i = 0; i < bones_count; i++)
			{
				auto src = model->get_bone(i);
				auto& dst = bones[i];
				auto name = std::string(src->get_name());
				auto e = entity->find_child(name);
				fassert(e);
				dst.name = name;
				dst.node = e->get_component_i<cNodePrivate>(0);
				fassert(dst.node);
				dst.offmat = src->get_offset_matrix();
			}
		}

		if (bones.empty() || animation_names.empty())
			return;

		auto ppath = entity->get_src(src_id).parent_path();
		auto sp = SUW::split(animation_names, ';');
		for (auto& s : sp)
		{
			auto fn = std::filesystem::path(s);
			if (!fn.is_absolute())
				fn = ppath / fn;

			auto animation = graphics::Animation::get(fn.c_str());
			if (animation)
			{
				auto& a = actions.emplace_back();
				a.total_frame = 0;

				auto chs = animation->get_channels_count();
				for (auto i = 0; i < chs; i++)
				{
					auto ch = animation->get_channel(i);
					auto find_bone = [&](const std::string& name) {
						for (auto i = 0; i < bones.size(); i++)
						{
							if (bones[i].name == name)
								return i;
						}
						return -1;
					};
					auto bid = find_bone(ch->get_node_name());
					if (bid != -1)
					{
						auto count = ch->get_keys_count();
						if (a.total_frame == 0)
							a.total_frame = count;
						else
							fassert(a.total_frame == count);

						auto& t = a.tracks.emplace_back();
						t.first = bid;
						t.second.resize(count);
						memcpy(t.second.data(), ch->get_keys(), sizeof(graphics::BoneKey) * count);
					}
				}
			}
		}
	}

	void cArmaturePrivate::advance()
	{
		auto& a = actions[playing];
		peeding_pose = { playing, frame };

		frame_accumulate += speed;
		while (frame_accumulate > 1.f)
		{
			frame++;
			if (frame == a.total_frame)
				frame = loop ? 0 : -1;

			for (auto& cb : callbacks)
				cb->call(frame);

			if (frame == -1)
				break;
			
			frame_accumulate -= 1.f;
		}
	}

	void cArmaturePrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (!first)
			return;
		if (!bones.empty())
		{
			if (peeding_pose.first != -1)
			{
				auto& a = actions[peeding_pose.first];
				a.apply(bones.data(), peeding_pose.second);
				peeding_pose.first = -1;
			}
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& b = bones[i];
				b.node->update_transform();
				bone_mats[i] = b.node->transform * b.offmat;
			}
			armature_id = s_renderer->add_mesh_armature(bones.size(), bone_mats.data());
		}
	}

	void cArmaturePrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);
	}

	void cArmaturePrivate::on_removed()
	{
		node = nullptr;
	}

	void cArmaturePrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		apply_src();
	}

	void cArmaturePrivate::on_left_world()
	{
		s_renderer = nullptr;
		stop();
	}

	cArmature* cArmature::create(void* parms)
	{
		return new cArmaturePrivate();
	}
}
