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

	void cArmaturePrivate::set_model_path(const std::filesystem::path& path)
	{
		if (model_path == path)
			return;
		bones.clear();
		model_path = path;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		data_changed(S<"model_path"_h>);
	}

	void cArmaturePrivate::set_animation_paths(const std::wstring& paths)
	{
		if (animation_paths == paths)
			return;
		animation_paths = paths;
		apply_src();
		if (node)
			node->mark_transform_dirty();
		data_changed(S<"animation_paths"_h>);
	}

	void cArmaturePrivate::play(uint id, float _speed, bool _loop)
	{
		speed = _speed;
		loop = _loop;

		if (animation_id == id)
			return;
		animation_id = id;
		frame = 0;
		frame_accumulate = 0.f;
		if (!event)
		{
			event = add_event([this]() {
				advance();
				if (frame != -1)
					return true;
				event = nullptr;
				stop();
				return false;
			});
		}
	}

	void cArmaturePrivate::stop()
	{
		animation_id = -1;
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
		auto ppath = entity->sources[source_id].parent_path();

		if (bones.empty() && entity)
		{
			graphics::Model* model = nullptr;
			auto fn = model_path;
			if (fn.extension().empty())
				model = graphics::Model::get(fn);
			else
			{
				if (!fn.is_absolute() && source_id != -1)
				{
					fn = ppath / fn;
					fn.make_preferred();
				}
				else
					fn = Path::get(fn);
				model = graphics::Model::get(fn);
			}
			assert(model);

			bones.resize(model->bones.size());
			bone_mats.resize(bones.size());
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& src = model->bones[i];
				auto& dst = bones[i];
				auto name = src.name;
				auto e = entity->find_child(name);
				assert(e);
				dst.name = name;
				dst.node = e->get_component_i<cNodePrivate>(0);
				assert(dst.node);
				dst.offmat = src.offset_matrix;
			}
		}

		if (bones.empty() || animation_paths.empty())
			return;

		auto sp = SUW::split(animation_paths, ';');
		for (auto& s : sp)
		{
			auto fn = std::filesystem::path(s);
			if (!fn.is_absolute())
				fn = ppath / fn;

			auto animation = graphics::Animation::get(fn);
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
							assert(a.total_frame == count);

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
		auto& a = actions[animation_id];
		peeding_pose = { animation_id, frame };

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
		assert(node);
	}

	void cArmaturePrivate::on_removed()
	{
		node = nullptr;
	}

	void cArmaturePrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		apply_src();
	}

	void cArmaturePrivate::on_left_world()
	{
		s_renderer = nullptr;
		stop();
	}

	cArmature* cArmature::create()
	{
		return new cArmaturePrivate();
	}
}
