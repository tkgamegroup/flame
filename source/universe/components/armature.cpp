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
									t.channel = &ch;
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

		if (default_animation)
			play(default_animation);
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
					auto& ch = *t.channel;
					if (!ch.position_keys.empty())
					{
						b.pose.p = mix(b.pose.p, ch.position_keys.front().p, transition_time / transition_duration);
						b.node->set_pos(b.pose.p);
					}
					if (!ch.rotation_keys.empty())
					{
						b.pose.q = slerp(b.pose.q, ch.rotation_keys.front().q, transition_time / transition_duration);
						b.node->set_qut(b.pose.q);
					}
					if (!ch.scaling_keys.empty())
					{
						b.pose.s = mix(b.pose.s, ch.scaling_keys.front().s, transition_time / transition_duration);
						b.node->set_scl(b.pose.s);
					}
				}

				transition_time += delta_time * speed;
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
					auto& ch = *t.channel;
					if (!ch.position_keys.empty())
					{
						auto rit = std::lower_bound(ch.position_keys.begin(), ch.position_keys.end(), playing_time, [](const auto& i, auto v) {
							return i.t < v;
						});
						auto lit = rit;
						if (lit != ch.position_keys.begin())
							lit--;
						if (lit == rit)
							b.pose.p = lit->p;
						else
							b.pose.p = mix(lit->p, rit->p, (playing_time - lit->t) / (rit->t - lit->t));
						b.node->set_pos(b.pose.p);
					}
					if (!ch.rotation_keys.empty())
					{
						auto rit = std::lower_bound(ch.rotation_keys.begin(), ch.rotation_keys.end(), playing_time, [](const auto& i, auto v) {
							return i.t < v;
						});
						auto lit = rit;
						if (lit != ch.rotation_keys.begin())
							lit--;
						if (lit == rit)
							b.pose.q = lit->q;
						else
							b.pose.q = slerp(lit->q, rit->q, (playing_time - lit->t) / (rit->t - lit->t));
						b.node->set_qut(b.pose.q);
					}
					if (!ch.scaling_keys.empty())
					{
						auto rit = std::lower_bound(ch.scaling_keys.begin(), ch.scaling_keys.end(), playing_time, [](const auto& i, auto v) {
							return i.t < v;
						});
						auto lit = rit;
						if (lit != ch.scaling_keys.begin())
							lit--;
						if (lit == rit)
							b.pose.s = lit->s;
						else
							b.pose.s = mix(lit->s, rit->s, (playing_time - lit->t) / (rit->t - lit->t));
						b.node->set_scl(b.pose.s);
					}
				}

				playing_time += delta_time * speed;
				if (playing_time >= a.duration)
				{
					if (!loop)
					{
						playing_callbacks.call("end"_h, playing_name);
						if (default_animation)
						{
							loop = true;
							play(default_animation);
						}
						else
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
				transition_duration = _it->second * speed;
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
