#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "armature_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	std::filesystem::path parse_name(const std::filesystem::path& src)
	{
		auto s = src.wstring();
		auto sp = SUW::split(s, '#');
		return Path::get(sp.empty() ? L"" : sp.front());
	}

	cArmaturePrivate::~cArmaturePrivate()
	{
		if (auto name = parse_name(armature_name); !name.empty())
			AssetManagemant::release(Path::get(name));
		if (model)
			graphics::Model::release(model);
		for (auto& a : animations)
		{
			if (!a.second.path.empty())
				AssetManagemant::release(Path::get(a.second.path));
			if (a.second.animation)
				graphics::Animation::release(a.second.animation);
		}
	}

	void cArmaturePrivate::attach()
	{
		if (model)
		{
			bones.resize(model->bones.size());
			for (auto i = 0; i < bones.size(); i++)
			{
				auto& src = model->bones[i];
				auto& dst = bones[i];
				auto e = entity->find_child_recursively(src.name);
				if (e)
				{
					dst.name = src.name;
					dst.node = e->get_component<cNodeT>();
					if (dst.node)
					{
						dst.offmat = src.offset_matrix;
						dst.node->data_listeners.add([this, i](uint hash) {
							if (hash == "transform"_h)
							{
								auto& bone = bones[i];
								bone.pose.m = bone.node->transform * bone.offmat;
								dirty = true;
							}
						}, "armature"_h);
					}
					else
						dst.offmat = mat4(1.f);
				}
				else
					printf("cArmature: cannot find node of bone's name: %s\n", src.name.c_str());
			}

			bone_node_map.clear();
			for (auto& c : entity->children)
			{
				Bone* pb = nullptr;
				if (!bones.empty())
					pb = &bones[0];
				for (auto& b : bones)
				{
					if (!b.node)
						continue;
					if (c->name.find(b.name) != std::string::npos)
					{
						pb = &b;
						break;
					}
				}
				bone_node_map[c->get_component<cNodeT>()] = pb;
			}

			if (!bones.empty())
			{
				for (auto& n : animation_names)
				{
					if (!n.first.empty() && !n.second.empty())
					{
						auto& a = animations[sh(n.second.c_str())];
						a.path = Path::get(n.first);
						AssetManagemant::get(a.path);
						a.animation = graphics::Animation::get(a.path);
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
										auto sp = SUS::split(bones[i].name, ':');
										if (sp.size() == 2 && sp[1] == name)
											return i;
									}
									return -1;
								};
								auto id = find_bone(ch.node_name);
								if (id == -1)
								{
									auto sp = SUS::split(ch.node_name, ':');
									if (sp.size() == 2)
										id = find_bone(sp[1]);
								}
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
									if (!t.positions.empty() && t.positions.back().first < a.duration)
									{
										auto k = t.positions.front();
										k.first = a.duration;
										t.positions.push_back(k);
									}
									t.rotations.resize(ch.rotation_keys.size());
									for (auto i = 0; i < t.rotations.size(); i++)
									{
										t.rotations[i].first = ch.rotation_keys[i].t;
										t.rotations[i].second = ch.rotation_keys[i].q;
									}
									if (!t.rotations.empty() && t.rotations.back().first < a.duration)
									{
										auto k = t.rotations.front();
										k.first = a.duration;
										t.rotations.push_back(k);
									}
									t.scalings.resize(ch.scaling_keys.size());
									for (auto i = 0; i < t.scalings.size(); i++)
									{
										t.scalings[i].first = ch.scaling_keys[i].t;
										t.scalings[i].second = ch.scaling_keys[i].s;
									}
									if (!t.scalings.empty() && t.scalings.back().first < a.duration)
									{
										auto k = t.scalings.front();
										k.first = a.duration;
										t.scalings.push_back(k);
									}
									//if (id == 0)
									//{
									//	if (!t.positions.empty())
									//	{
									//		auto first_xz = t.positions.front().second.xz();
									//		for (auto i = 1; i < t.positions.size(); i++)
									//			t.positions[i].second.xz = first_xz;
									//	}
									//}
								}
							}
						}
					}
				}

				for (auto& t : animation_transitions)
				{
					auto it = animations.find(sh(std::get<0>(t)));
					if (it != animations.end())
						it->second.transitions[sh(std::get<1>(t))] = std::get<2>(t);
				}
			}
		}
	}

	void cArmaturePrivate::detach()
	{
		for (auto& b : bones)
		{
			if (b.node)
				b.node->data_listeners.remove("armature"_h);
		}
		bones.clear();
		for (auto& a : animations)
		{
			if (!a.second.path.empty())
				AssetManagemant::release(Path::get(a.second.path));
			if (a.second.animation)
				graphics::Animation::release(a.second.animation);
		}
		animations.clear();
	}

	void cArmaturePrivate::update_instance()
	{
		if (dirty)
		{
			std::vector<mat4> mats(bones.size());
			for (auto i = 0; i < bones.size(); i++)
				mats[i] = bones[i].pose.m;
			sRenderer::instance()->set_armature_instance(instance_id, mats.data(), mats.size());
			dirty = false;
		}
	}

	void cArmaturePrivate::on_active()
	{
		attach();

		if (instance_id != 0)
			instance_id = sRenderer::instance()->register_armature_instance(-1);

		node->mark_transform_dirty();

		if (auto_play)
		{
			if (!animation_names.empty())
				play(sh(animation_names[0].second.c_str()));
		}
	}

	void cArmaturePrivate::on_inactive()
	{
		stop();

		detach();

		if (instance_id != 0)
		{
			sRenderer::instance()->register_armature_instance(instance_id);
			instance_id = -1;
		}
	}

	void cArmaturePrivate::update()
	{
		if (playing_name != 0)
		{
			auto& a = animations[playing_name];
			if (transition_time >= 0.f)
			{
				for (auto& t : a.tracks)
				{
					auto& b = bones[t.bone_idx];
					if (!t.positions.empty())
					{
						b.pose.p = mix(b.pose.p, t.positions.front().second, transition_time / transition_duration);
						b.node->set_pos(b.pose.p);
					}
					if (!t.rotations.empty())
					{
						b.pose.q = slerp(b.pose.q, t.rotations.front().second, transition_time / transition_duration);
						b.node->set_qut(b.pose.q);
					}
					if (!t.scalings.empty())
					{
						b.pose.s = mix(b.pose.s, t.scalings.front().second, transition_time / transition_duration);
						b.node->set_scl(b.pose.s);
					}
				}

				transition_time += delta_time * playing_speed;
				if (transition_time >= transition_duration)
				{
					playing_time = transition_time - transition_duration;
					transition_time = -1.f;
				}
			}
			else
			{
				for (auto& t : a.tracks)
				{
					auto& b = bones[t.bone_idx];
					if (!t.positions.empty())
					{
						auto rit = std::lower_bound(t.positions.begin(), t.positions.end(), playing_time, [](const auto& i, auto v) {
							return i.first < v;
						});
						auto lit = rit;
						if (lit != t.positions.begin())
							lit--;
						if (lit == rit)
							b.pose.p = lit->second;
						else
							b.pose.p = mix(lit->second, rit->second, (playing_time - lit->first) / (rit->first - lit->first));
						b.node->set_pos(b.pose.p);
					}
					if (!t.rotations.empty())
					{
						auto rit = std::lower_bound(t.rotations.begin(), t.rotations.end(), playing_time, [](const auto& i, auto v) {
							return i.first < v;
						});
						auto lit = rit;
						if (lit != t.rotations.begin())
							lit--;
						if (lit == rit)
							b.pose.q = lit->second;
						else
							b.pose.q = slerp(lit->second, rit->second, (playing_time - lit->first) / (rit->first - lit->first));
						b.node->set_qut(b.pose.q);
					}
					if (!t.scalings.empty())
					{
						auto rit = std::lower_bound(t.scalings.begin(), t.scalings.end(), playing_time, [](const auto& i, auto v) {
							return i.first < v;
						});
						auto lit = rit;
						if (lit != t.scalings.begin())
							lit--;
						if (lit == rit)
							b.pose.s = lit->second;
						else
							b.pose.s = mix(lit->second, rit->second, (playing_time - lit->first) / (rit->first - lit->first));
						b.node->set_scl(b.pose.s);
					}
				}

				playing_time += delta_time * playing_speed;
				if (playing_time >= a.duration)
				{
					if (!loop)
					{
						playing_callbacks.call("end"_h, playing_name);
						stop();
					}
					else
						playing_time = fmod(playing_time, a.duration);
				}
			}
		}
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
				AssetManagemant::release(Path::get(name));
			if (!_name.empty())
				AssetManagemant::get(Path::get(_name));
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

		if (instance_id != -1)
		{
			stop();
			detach();
			attach();
		}

		node->mark_transform_dirty();
		data_changed("armature_name"_h);
	}

	void cArmaturePrivate::set_animation_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names)
	{
		animation_names = names;

		if (instance_id != -1)
		{
			stop();
			detach();
			attach();
		}
	}

	void cArmaturePrivate::reset()
	{
		if (!bones.empty())
		{
			for (auto& b : bones)
				b.pose.m = mat4(1.f);
		}
		dirty = true;
	}

	void cArmaturePrivate::play(uint name)
	{
		if (playing_name == name)
			return;

		transition_time = -1.f;
		auto it = animations.find(playing_name);
		if (it != animations.end())
		{
			auto _it = it->second.transitions.find(name);
			if (_it != it->second.transitions.end())
			{
				transition_duration = _it->second * playing_speed;
				transition_time = 0.f;
			}
		}

		playing_name = name;
		playing_time = 0;
	}

	void cArmaturePrivate::stop()
	{
		playing_name = 0;
		playing_time = 0;
		transition_time = -1.f;
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
