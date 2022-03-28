#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "armature_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	const auto TransitionDuration = 0.3f;

	mat4 cArmaturePrivate::Bone::calc_mat()
	{
		if (!node)
			return mat4(1.f);
		return node->transform * offmat;
	}

	cArmaturePrivate::~cArmaturePrivate()
	{
		node->drawers.remove("armature"_h);
		node->measurers.remove("armature"_h);
	}

	void cArmaturePrivate::on_init()
	{
		node->drawers.add([this](sRendererPtr renderer) {
			draw(renderer);
		}, "armature"_h);

		node->measurers.add([this](AABB* ret) {
			if (!model)
				return false;
			*ret = AABB(model->bounds.get_points(node->transform));
			return true;
		}, "armature"_h);

		node->mark_transform_dirty();
	}

	void cArmaturePrivate::set_model_name(const std::filesystem::path& _model_name)
	{
		if (model_name == _model_name)
			return;
		if (!model_name.empty())
			AssetManagemant::release_asset(Path::get(model_name));
		model_name = _model_name;
		if (!model_name.empty())
			AssetManagemant::get_asset(Path::get(model_name));
		bones.clear();

		graphics::ModelPtr _model = nullptr;
		if (!model_name.empty())
			_model = graphics::Model::get(model_name);
		if (model != _model)
		{
			if (model)
				graphics::Model::release(model);
			model = _model;
		}
		else if (_model)
			graphics::Model::release(_model);
		bones_dirty = true;

		if (node)
			node->mark_transform_dirty();
		data_changed("model_name"_h);
	}

	void cArmaturePrivate::set_animation_names(const std::wstring& paths)
	{
		if (animation_names == paths)
			return;
		animation_names = paths;

		animations_dirty = true;

		if (node)
			node->mark_transform_dirty();
		data_changed("animation_names"_h);
	}

	void cArmaturePrivate::play(uint id)
	{
		if (playing_id == id)
			return;
		stop();
		playing_id = id;
	}

	void cArmaturePrivate::stop()
	{
		playing_id = -1;
		playing_time = 0;
		transition_time = -1.f;
	}

	void cArmaturePrivate::draw(sRendererPtr renderer)
	{
		if (instance_id == -1)
			return;

		if (bones_dirty)
		{
			bones.clear();

			if (model)
			{
				bones.resize(model->bones.size());
				for (auto i = 0; i < bones.size(); i++)
				{
					auto& src = model->bones[i];
					auto& dst = bones[i];
					auto name = src.name;
					auto e = entity->find_child(name);
					if (e)
					{
						dst.name = name;
						dst.node = e->get_component_i<cNodeT>(0);
						if (dst.node)
							dst.offmat = src.offset_matrix;
					}
				}
			}

			bones_dirty = false;
			animations_dirty = true;
		}
		if (animations_dirty)
		{
			stop();
			animations.clear();

			if (!bones.empty() && !animation_names.empty())
			{
				auto sp = SUW::split(animation_names, ';');
				for (auto& s : sp)
				{
					auto animation = graphics::Animation::get(s);
					if (animation)
					{
						auto& a = animations.emplace_back();
						a.duration = animation->duration;

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
								auto& t = a.tracks.emplace_back();
								t.bone_idx = id;
								t.positions.resize(ch.position_keys.size());
								for (auto i = 0; i < t.positions.size(); i++)
								{
									t.positions[i].first = ch.position_keys[i].t;
									t.positions[i].second = ch.position_keys[i].p;
								}
								t.rotations.resize(ch.rotation_keys.size());
								for (auto i = 0; i < t.rotations.size(); i++)
								{
									t.rotations[i].first = ch.rotation_keys[i].t;
									t.rotations[i].second = ch.rotation_keys[i].q;
								}
							}
						}
					}
				}
			}

			animations_dirty = false;
		}

		if (frame < (int)frames)
		{
			if (playing_id != -1)
			{
				auto& a = animations[playing_id];
				if (transition_time > 0.f)
				{
					for (auto& t : a.tracks)
					{
						auto& b = bones[t.bone_idx];
						if (!t.positions.empty())
						{
							b.pose.p = mix(b.pose.p, t.positions.front().second, transition_time / (t.positions.front().first + TransitionDuration));
							b.node->set_pos(b.pose.p);
						}
						if (!t.rotations.empty())
						{
							b.pose.q = mix(b.pose.q, t.rotations.front().second, transition_time / (t.rotations.front().first + TransitionDuration));
							b.node->set_qut(b.pose.q);
						}
					}

					transition_time += delta_time * playing_speed;
					if (transition_time >= TransitionDuration)
						transition_time = -1.f;
				}
				else
				{
					for (auto& t : a.tracks)
					{
						auto& b = bones[t.bone_idx];
						if (!t.positions.empty())
						{
							auto lit = std::upper_bound(t.positions.begin(), t.positions.end(), playing_time, [](auto v, const auto& i) {
								return v < i.first;
							});
							if (lit == t.positions.end())
								lit--;
							auto rit = lit + 1;
							if (rit == t.positions.end())
								rit--;
							if (lit == rit)
								b.pose.p = lit->second;
							b.pose.p = mix(lit->second, rit->second, (playing_time - lit->first) / (rit->first - lit->first));
							b.node->set_pos(b.pose.p);
						}
						if (!t.rotations.empty())
						{
							auto lit = std::upper_bound(t.rotations.begin(), t.rotations.end(), playing_time, [](auto v, const auto& i) {
								return v < i.first;
							});
							if (lit == t.rotations.end())
								lit--;
							auto rit = lit + 1;
							if (rit == t.rotations.end())
								rit--;
							if (lit == rit)
								b.pose.q = lit->second;
							b.pose.q = mix(lit->second, rit->second, (playing_time - lit->first) / (rit->first - lit->first));
							b.node->set_qut(b.pose.q);
						}
					}

					playing_time += delta_time * playing_speed;
					if (playing_time >= a.duration)
					{
						if (!loop)
							stop();
						else
							playing_time = fmod(playing_time, a.duration);
					}
				}
			}

			auto dst = renderer->set_armature_instance(instance_id);
			for (auto i = 0; i < bones.size(); i++)
				dst[i] = bones[i].calc_mat();

			frame = frames;
		}
	}

	void cArmaturePrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_armature_instance(-1);

		node->mark_transform_dirty();
	}

	void cArmaturePrivate::on_inactive()
	{
		stop();
		bones.clear();
		animations.clear();

		sRenderer::instance()->register_armature_instance(instance_id);
		instance_id = -1;
	}

	struct cArmatureCreate : cArmature::Create
	{
		cArmaturePtr operator()(EntityPtr e) override
		{
			return new cArmaturePrivate();
		}
	}cArmature_create;
	cArmature::Create& cArmature::create = cArmature_create;
}
