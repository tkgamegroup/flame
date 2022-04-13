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

	std::filesystem::path parse_name(const std::filesystem::path& src)
	{
		auto sp = SUW::split(src.wstring(), '#');
		return Path::get(sp.empty() ? L"" : sp.front());
	}

	cArmaturePrivate::~cArmaturePrivate()
	{
		node->drawers.remove("armature"_h);
		node->measurers.remove("armature"_h);

		if (auto name = parse_name(armature_name); !name.empty())
			AssetManagemant::release_asset(Path::get(name));
		if (model)
			graphics::Model::release(model);
		for (auto& a : animations)
		{
			if (!a.second.path.empty())
				AssetManagemant::release_asset(Path::get(a.second.path));
			if (a.second.animation)
				graphics::Animation::release(a.second.animation);
		}
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

	void cArmaturePrivate::set_armature_name(const std::filesystem::path& _armature_name)
	{
		if (armature_name == _armature_name)
			return;

		auto name = parse_name(armature_name);
		auto _name = parse_name(_armature_name);
		armature_name = _armature_name;

		if (name != _name)
		{
			if (!name.empty())
				AssetManagemant::release_asset(Path::get(name));
			if (!_name.empty())
				AssetManagemant::get_asset(Path::get(_name));
		}

		graphics::ModelPtr _model = nullptr;
		if (!armature_name.empty())
			_model = graphics::Model::get(armature_name);
		if (model != _model)
		{
			if (model)
				graphics::Model::release(model);
			model = _model;
		}
		else if (_model)
			graphics::Model::release(_model);

		bones.clear();

		if (instance_id != -1)
		{
			on_inactive();
			on_active();
		}

		node->mark_transform_dirty();
		data_changed("armature_name"_h);
	}

	void cArmaturePrivate::bind_animation(uint name_hash, const std::filesystem::path& animation_path)
	{
		if (bones.empty())
			return;

		auto& a = animations[name_hash];
		if (a.path != animation_path)
		{
			if (!a.path.empty())
				AssetManagemant::release_asset(Path::get(a.path));
			a.path = animation_path;
			if (!a.path.empty())
				AssetManagemant::get_asset(Path::get(a.path));
		}
		auto _animation = graphics::Animation::get(animation_path);
		if (a.animation != _animation)
		{
			if (a.animation)
				graphics::Animation::release(a.animation);
			a.animation = _animation;
		}
		else if (_animation)
			graphics::Animation::release(_animation);
		if (a.animation)
		{
			a.duration = a.animation->duration;

			for (auto& ch : a.animation->channels)
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

	void cArmaturePrivate::unbind_animation(uint name_hash)
	{
		auto it = animations.find(name_hash);
		if (it == animations.end())
			return;

		if (!it->second.path.empty())
			AssetManagemant::release_asset(Path::get(it->second.path));
		if (it->second.animation)
			graphics::Animation::release(it->second.animation);
		animations.erase(it);
	}

	void cArmaturePrivate::play(uint name)
	{
		if (playing_name == name)
			return;
		stop();
		playing_name = name;
	}

	void cArmaturePrivate::stop()
	{
		playing_name = 0;
		playing_time = 0;
		transition_time = -1.f;
	}

	void cArmaturePrivate::draw(sRendererPtr renderer)
	{
		if (instance_id == -1)
			return;

		if (frame < (int)frames)
		{
			if (playing_name != 0)
			{
				auto& a = animations[playing_name];
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
							{
								if (loop)
									rit = t.positions.begin();
								else
									rit = lit;
							}
							if (lit == rit)
								b.pose.p = lit->second;
							else
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
							{
								if (loop)
									rit = t.rotations.begin();
								else
									rit = lit;
							}
							if (lit == rit)
								b.pose.q = lit->second;
							else
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
					else
						dst.offmat = mat4(1.f);
				}
				else
					printf("cArmature: cannot find node of bone's name: %s\n", name.c_str());
			}
		}

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
