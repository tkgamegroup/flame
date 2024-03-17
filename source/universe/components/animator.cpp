#include "../../foundation/blueprint.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "animator_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cAnimatorPrivate::~cAnimatorPrivate()
	{
		if (!armature_name.empty())
			AssetManagemant::release(Path::get(armature_name));
		if (armature)
			graphics::Armature::release(armature);
		for (auto& a : animations)
		{
			if (!a.second.path.empty())
				AssetManagemant::release(Path::get(a.second.path));
			if (a.second.animation)
				graphics::Animation::release(a.second.animation);
		}
	}

	void cAnimatorPrivate::new_cluster(uint cluster_idx, const std::string& name, EntityPtr e, const mat4& offmat)
	{
		auto& dst = clusters[cluster_idx];
		dst.name = name;
		dst.node = e->get_component<cNodeT>();
		if (dst.node)
		{
			dst.offmat = offmat;
			dst.node->data_listeners.add([this, cluster_idx](uint hash) {
				if (hash == "transform"_h)
				{
					auto& cluster = clusters[cluster_idx];
					cluster.pose.m = cluster.node->transform * cluster.offmat;
					dirty = true;
				}
			}, "animator"_h);
		}
		else
			dst.offmat = mat4(1.f);
	}

	void cAnimatorPrivate::attach()
	{
		if (armature)
		{
			clusters.resize(armature->bones.size());
			for (auto i = 0; i < clusters.size(); i++)
			{
				auto& bone = armature->bones[i];
				if (auto e = entity->find_child_recursively(bone.name); e)
					new_cluster(i, bone.name, e, bone.offset_matrix);
				else
					printf("cAnimator: cannot find node of bone's name: %s\n", bone.name.c_str());
			}
		}
		else
		{
			auto all_nodes = entity->get_components<cNode>(-1);
			clusters.resize(all_nodes.size());
			for (auto i = 0; i < clusters.size(); i++)
			{
				auto node = all_nodes[i];
				new_cluster(i, node->entity->name, node->entity, mat4(1.f));
			}
		}

		node_to_cluster.clear();
		for (auto& c : entity->children)
		{
			Cluster* pc = nullptr;
			if (!clusters.empty())
				pc = &clusters[0];
			for (auto& cluster : clusters)
			{
				if (!cluster.node)
					continue;
				if (c->name.find(cluster.name) != std::string::npos)
				{
					pc = &cluster;
					break;
				}
			}
			node_to_cluster[c->get_component<cNodeT>()] = pc;
		}

		if (!clusters.empty())
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
							auto find_cluster = [&](std::string_view name) {
								for (auto i = 0; i < clusters.size(); i++)
								{
									if (clusters[i].name == name)
										return i;
									auto sp = SUS::split(clusters[i].name, ':');
									if (sp.size() == 2 && sp[1] == name)
										return i;
								}
								return -1;
							};
							auto id = find_cluster(ch.node_name);
							if (id == -1)
							{
								auto sp = SUS::split(ch.node_name, ':');
								if (sp.size() == 2)
									id = find_cluster(sp[1]);
							}
							if (id != -1)
							{
								auto& t = a.tracks.emplace_back();
								t.cluster_idx = id;
								t.channel = &ch;
							}
						}

						a.events_beg = a.animation->events.begin();
						a.events_end = a.animation->events.end();
						a.events_it = a.events_beg;
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

	void cAnimatorPrivate::detach()
	{
		for (auto& c : clusters)
		{
			if (c.node)
				c.node->data_listeners.remove("animator"_h);
		}
		clusters.clear();
		for (auto& a : animations)
		{
			if (!a.second.path.empty())
				AssetManagemant::release(Path::get(a.second.path));
			if (a.second.animation)
				graphics::Animation::release(a.second.animation);
		}
		animations.clear();
	}

	void cAnimatorPrivate::update_instance()
	{
		if (dirty)
		{
			if (armature)
			{
				std::vector<mat4> mats(clusters.size());
				for (auto i = 0; i < clusters.size(); i++)
					mats[i] = clusters[i].pose.m;
				sRenderer::instance()->set_armature_instance(instance_id, mats.data(), mats.size());
			}
			dirty = false;
		}
	}

	void cAnimatorPrivate::on_active()
	{
		attach();

		if (instance_id != 0 && armature)
			instance_id = sRenderer::instance()->register_armature_instance(-1);

		node->mark_transform_dirty();

		if (default_animation)
			play(default_animation);
	}

	void cAnimatorPrivate::on_inactive()
	{
		stop();

		detach();

		if (instance_id != 0)
		{
			sRenderer::instance()->register_armature_instance(instance_id);
			instance_id = -1;
		}
	}

	void cAnimatorPrivate::update()
	{
		if (playing_name != 0)
		{
			auto& a = animations[playing_name];
			if (transition_time >= 0.f)
			{
				for (auto& t : a.tracks)
				{
					auto& c = clusters[t.cluster_idx];
					auto& ch = *t.channel;
					if (!ch.position_keys.empty())
					{
						c.pose.p = mix(c.pose.p, ch.position_keys.front().p, transition_time / transition_duration);
						c.node->set_pos(c.pose.p);
					}
					if (!ch.rotation_keys.empty())
					{
						c.pose.q = slerp(c.pose.q, ch.rotation_keys.front().q, transition_time / transition_duration);
						c.node->set_qut(c.pose.q);
					}
					if (!ch.scaling_keys.empty())
					{
						c.pose.s = mix(c.pose.s, ch.scaling_keys.front().s, transition_time / transition_duration);
						c.node->set_scl(c.pose.s);
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
					auto& c = clusters[t.cluster_idx];
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
							c.pose.p = lit->p;
						else
							c.pose.p = mix(lit->p, rit->p, (playing_time - lit->t) / (rit->t - lit->t));
						c.node->set_pos(c.pose.p);
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
							c.pose.q = lit->q;
						else
							c.pose.q = slerp(lit->q, rit->q, (playing_time - lit->t) / (rit->t - lit->t));
						c.node->set_qut(c.pose.q);
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
							c.pose.s = lit->s;
						else
							c.pose.s = mix(lit->s, rit->s, (playing_time - lit->t) / (rit->t - lit->t));
						c.node->set_scl(c.pose.s);
					}
				}

				if (a.events_it != a.events_end)
				{
					if (playing_time >= a.events_it->t)
					{
						callbacks.call(a.events_it->name_hash, playing_name);
						for (auto g : bp_callbacks)
						{
							voidptr inputs[2];
							inputs[0] = &a.events_it->name_hash;
							inputs[1] = &playing_name;
							g->instance->call(g, inputs, nullptr);
						}

						a.events_it++;
					}
				}

				playing_time += delta_time * speed;
				if (playing_time >= a.duration)
				{
					if (!loop)
					{
						callbacks.call("end"_h, playing_name);
						for (auto g : bp_callbacks)
						{
							voidptr inputs[2]; uint ev = "end"_h;
							inputs[0] = &ev;
							inputs[1] = &playing_name;
							g->instance->call(g, inputs, nullptr);
						}

						if (default_animation)
						{
							loop = true;
							play(default_animation);
						}
						else
							stop();
					}
					else
					{
						playing_time = fmod(playing_time, a.duration);
						a.events_it = a.events_beg;
					}
				}
			}
		}
	}

	void cAnimatorPrivate::set_armature_name(const std::filesystem::path& _armature_name)
	{
		if (armature_name == _armature_name)
			return;

		if (armature_name != _armature_name)
		{
			if (!armature_name.empty())
				AssetManagemant::release(Path::get(armature_name));
			if (!_armature_name.empty())
				AssetManagemant::get(Path::get(_armature_name));
		}

		graphics::ArmaturePtr _armature = nullptr;
		if (!_armature_name.empty())
			_armature = graphics::Armature::get(_armature_name);
		if (armature != _armature)
		{
			if (armature)
				graphics::Armature::release(armature);
			armature = _armature;
		}
		else if (_armature)
			graphics::Armature::release(_armature);

		armature_name = _armature_name;

		if (!clusters.empty())
		{
			stop();
			detach();
			attach();
		}

		node->mark_transform_dirty();
		data_changed("armature_name"_h);
	}

	void cAnimatorPrivate::set_animation_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names)
	{
		animation_names = names;

		if (!clusters.empty())
		{
			stop();
			detach();
			attach();
		}
	}

	void cAnimatorPrivate::reset()
	{
		if (!clusters.empty())
		{
			for (auto& c : clusters)
				c.pose.m = mat4(1.f);
		}
		dirty = true;
	}

	void cAnimatorPrivate::play(uint name)
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

			it->second.events_it = it->second.events_beg;
		}

		playing_name = name;
		playing_time = 0;
	}

	void cAnimatorPrivate::stop()
	{
		playing_name = 0;
		playing_time = 0;
		transition_time = -1.f;
	}

	struct cAnimatorCreate : cAnimatorPrivate::Create
	{
		cAnimatorPtr operator()(EntityPtr e) override
		{
			return new cAnimatorPrivate();
		}
	}cAnimator_create;
	cAnimator::Create& cAnimator::create = cAnimator_create;
}
