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

	void cArmaturePrivate::set_animations(std::wstring_view _animation_names)
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

	void cArmaturePrivate::play(uint id, float _speed, bool _loop)
	{
		speed = _speed;
		loop = _loop;

		if (anim == id)
			return;
		anim = id;
		frame = 0;
		frame_accumulate = 0.f;
		if (!event)
		{
			event = add_event([](Capture& c) {
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
		anim = -1;
		if (event)
		{
			remove_event(event);
			event = nullptr;
		}
	}

	void cArmaturePrivate::stop_at(uint id, int frame)
	{
		stop();

		auto& a = actions[id];
		peeding_pose = { id, frame < 0 ? a.total_frame + frame : frame };
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
					auto find_bone = [&](std::string_view name) {
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
		auto& a = actions[anim];
		peeding_pose = { anim, frame };

		frame_accumulate += speed;
		while (frame_accumulate > 1.f)
		{
			frame_accumulate -= 1.f;

			frame++;
			if (frame == a.total_frame)
				frame = loop ? 0 : -1;

			if (frame == -1)
				break;
		}
	}

	void cArmaturePrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (first && !bones.empty())
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
