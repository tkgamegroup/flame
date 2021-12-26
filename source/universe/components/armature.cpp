#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "armature_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	void cArmaturePrivate::Action::apply(Bone* bones, uint frame)
	{
		for (auto& t : tracks)
		{
			auto& b = bones[t.first];
			auto& k = t.second[frame];
			b.node->set_pos(k.p);
			b.node->set_qut(k.q);
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
		if (bones.empty())
		{
			graphics::Model* model = nullptr;
			auto fn = model_path;
			if (fn.extension().empty())
				model = graphics::Model::get(fn);
			else
			{
				fn = Path::get(fn);
				model = graphics::Model::get(fn);
			}

			bones.resize(model->bones.size());
			bone_mats.resize(bones.size());
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& src = model->bones[i];
				auto& dst = bones[i];
				auto name = src.name;
				auto e = entity->find_child(name);
				if (e)
				{
					dst.name = name;
					dst.node = e->get_component_i<cNodePrivate>(0);
					if (dst.node)
						dst.offmat = src.offset_matrix;
				}
			}
		}

		if (bones.empty() || animation_paths.empty())
			return;

		auto sp = SUW::split(animation_paths, ';');
		for (auto& s : sp)
		{
			auto fn = std::filesystem::path(s);
			fn = Path::get(fn);

			auto animation = graphics::Animation::get(fn);
			if (animation)
			{
				auto& a = actions.emplace_back();
				a.total_frame = 0;

				for (auto& ch : animation->channels)
				{
					auto find_bone = [&](std::string_view name) {
						for (auto i = 0; i < bones.size(); i++)
						{
							if (bones[i].name == name)
								return i;
						}
						return -1;
					};
					auto id = find_bone(ch.node_name);
					if (id != -1)
					{
						uint count = ch.keys.size();
						if (a.total_frame == 0)
							a.total_frame = max(a.total_frame, count);

						auto& t = a.tracks.emplace_back();
						t.first = id;
						t.second.resize(count);
						memcpy(t.second.data(), ch.keys.data(), sizeof(graphics::Channel::Key) * count);
					}
				}

				for (auto& t : a.tracks)
					t.second.resize(a.total_frame);
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

	void cArmaturePrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		if (node)
		{
			drawer_lis = node->drawers.add([this](sNodeRendererPtr s_renderer, bool shadow_pass) {
				if (frame < frames)
				{
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
						armature_id = s_renderer->add_mesh_armature(bone_mats.data(), bone_mats.size());
					}
					frame = frames;
				}
			});
		}
	}

	void cArmaturePrivate::on_removed()
	{
		if (node)
		{
			node->drawers.remove(drawer_lis);
			node = nullptr;
		}
	}

	void cArmaturePrivate::on_entered_world()
	{
		apply_src();
	}

	void cArmaturePrivate::on_left_world()
	{
		stop();
	}

	struct cArmatureCreatePrivate : cArmature::Create
	{
		cArmaturePtr operator()() override
		{
			return new cArmaturePrivate();
		}
	}cArmature_create_private;
	cArmature::Create& cArmature::create = cArmature_create_private;
}
