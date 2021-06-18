#include "../../graphics/device.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "animation_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cAnimationPrivate::~cAnimationPrivate()
	{
		stop();
	}

	void cAnimationPrivate::set_model_name(const std::filesystem::path& name)
	{
		if (model_name == name)
			return;
		bones.clear();
		model_name = name;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cAnimationPrivate::set_src(const std::wstring& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"src"_h>);
	}

	void cAnimationPrivate::play(uint id)
	{
		if (playing == id)
			return;
		playing = id;
		frame = 0;
		if (!event)
		{
			event = looper().add_event([](Capture& c) {
				auto thiz = c.thiz<cAnimationPrivate>();
				thiz->advance();
				if (thiz->frame != -1)
					c._current = nullptr;
				else
				{
					thiz->event = nullptr;
					thiz->stop();
				}
			}, Capture().set_thiz(this), 1.f / 24.f);
		}
	}

	void cAnimationPrivate::stop()
	{
		playing = -1;
		if (event)
		{
			looper().remove_event(event);
			event = nullptr;
		}
		if (entity)
			entity->component_data_changed(this, S<"src"_h>);
	}

	void cAnimationPrivate::set_loop(bool l)
	{
		if (loop == l)
			return;
		loop = l;
	}

	void cAnimationPrivate::apply_src()
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

		if (bones.empty() || src.empty())
			return;

		auto ppath = entity->get_src(src_id).parent_path();
		auto sp = SUW::split(src, ';');
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
						auto pkc = ch->get_position_keys_count();
						auto rkc = ch->get_rotation_keys_count();
						a.total_frame = max(a.total_frame, max(pkc, rkc));

						auto& t = a.tracks.emplace_back();
						t.first = bid;
						t.second.resize(a.total_frame);

						auto pk = ch->get_position_keys();
						auto rk = ch->get_rotation_keys();
						for (auto j = 0; j < a.total_frame; j++)
						{
							auto& f = t.second[j];
							f.p = j < pkc ? pk[j].v : vec3(0.f);
							f.q = j < rkc ? rk[j].v : quat(1.f, 0.f, 0.f, 0.f);
						}
					}
				}

			}
		}
	}

	void cAnimationPrivate::advance()
	{
		auto& a = actions[playing];
		for (auto& t : a.tracks)
		{
			auto& b = bones[t.first];
			auto& k = t.second[frame];
			b.node->set_pos(k.p);
			b.node->set_quat(k.q);
		}
		frame++;
		if (frame == a.total_frame)
			frame = loop ? 0 : -1;
	}

	void cAnimationPrivate::draw(sRenderer* s_renderer)
	{
		if (!bones.empty())
		{
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& b = bones[i];
				b.node->update_transform();
				bone_mats[i] = b.node->transform * b.offmat;
			}
			armature_id = s_renderer->add_armature(bones.size(), bone_mats.data());
		}
	}

	void cAnimationPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		drawer = node->add_drawer([](Capture& c, sRendererPtr s_renderer) {
			auto thiz = c.thiz<cAnimationPrivate>();
			thiz->draw(s_renderer);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cAnimationPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	void cAnimationPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		apply_src();
	}

	void cAnimationPrivate::on_left_world()
	{
		s_renderer = nullptr;
		stop();
	}

	cAnimation* cAnimation::create(void* parms)
	{
		return f_new<cAnimationPrivate>();
	}
}
