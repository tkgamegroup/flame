#include "../../foundation/blueprint.h"
#include "../entity_private.h"
#include "../components/node_private.h"
#include "../components/element_private.h"
#include "../components/mesh_private.h"
#include "../components/skinned_mesh_private.h"
#include "../components/animator_private.h"
#include "../components/directional_light_private.h"
#include "../components/point_light_private.h"
#include "../components/bp_instance_private.h"
#include "tween_private.h"
#include "graveyard_private.h"
#include "renderer_private.h"

namespace flame
{
	sTweenPrivate::Animation::~Animation()
	{
		if (custom_data)
			custom_data_type->destroy(custom_data);
		if (bp_renderer_entity)
			bp_renderer_entity->remove_from_parent();
	}

	sTweenPrivate::Action& sTweenPrivate::Animation::new_action(float duration)
	{
		auto& t = *curr_track;
		auto& a = t.actions.emplace_back();
		a.target = curr_target;
		a.duration = duration;
		a.start_time = t.duration;
		a.end_time = a.start_time + duration;
		t.duration += duration;
		return a;
	}

	void sTweenPrivate::Animation::action_get_start_value(Action& a)
	{
		switch (a.type)
		{
		case ActionMove:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = node->pos;
				else if (auto element = a.target.e->get_component<cElement>(); element)
					(a.dir == ActionForward ? a.v0.f.xy : a.v1.f.xy) = element->pos;
			}
			else if (type == Tween3DTargets)
			{
				auto& tar = targets[a.target.idx]._3d;
				if (tar.pos)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = *tar.pos;
			}
			else if (type == Tween2DTargets)
			{
				auto& tar = targets[a.target.idx]._2d;
				if (tar.pos)
					(a.dir == ActionForward ? a.v0.f.xy : a.v1.f.xy) = *tar.pos;
			}
			else if (type == TweenBpCustomRenderer)
				(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = targets[a.target.idx].renderer_data.pos;
			break;
		case ActionRotate:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = node->get_eul();
				else if (auto element = a.target.e->get_component<cElement>(); element)
					(a.dir == ActionForward ? a.v0.f.x : a.v1.f.x) = element->ang;
			}
			else if (type == Tween3DTargets)
			{
				auto& tar = targets[a.target.idx]._3d;
				if (tar.eul)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = *tar.eul;
			}
			else if (type == Tween2DTargets)
			{
				auto& tar = targets[a.target.idx]._2d;
				if (tar.ang)
					(a.dir == ActionForward ? a.v0.f.x : a.v1.f.x) = *tar.ang;
			}
			else if (type == TweenBpCustomRenderer)
				(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = targets[a.target.idx].renderer_data.eul;
			break;
		case ActionScale:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = node->scl;
				else if (auto element = a.target.e->get_component<cElement>(); element)
					(a.dir == ActionForward ? a.v0.f.xy : a.v1.f.xy) = element->scl;
			}
			else if (type == Tween3DTargets)
			{
				auto& tar = targets[a.target.idx]._3d;
				if (tar.scl)
					(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = *tar.scl;
			}
			else if (type == Tween2DTargets)
			{
				auto& tar = targets[a.target.idx]._2d;
				if (tar.scl)
					(a.dir == ActionForward ? a.v0.f.xy : a.v1.f.xy) = *tar.scl;
			}
			else if (type == TweenBpCustomRenderer)
				(a.dir == ActionForward ? a.v0.f.xyz : a.v1.f.xyz) = targets[a.target.idx].renderer_data.scl;
			break;
		case ActionObjectColor:
			if (type == TweenEntity)
			{
				auto meshes = a.target.e->get_components<cMesh>(-1);
				if (!meshes.empty())
					(a.dir == ActionForward ? a.v0.c : a.v1.c) = meshes[0]->color;
				else
				{
					auto skinned_meshes = a.target.e->get_components<cSkinnedMesh>(-1);
					if (!skinned_meshes.empty())
						(a.dir == ActionForward ? a.v0.c : a.v1.c) = skinned_meshes[0]->color;
				}
			}
			break;
		case ActionLightColor:
			if (type == TweenEntity)
			{
				if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
					(a.dir == ActionForward ? a.v0.f : a.v1.f) = light->color;
				if (auto light = a.target.e->get_component<cPointLight>(); light)
					(a.dir == ActionForward ? a.v0.f : a.v1.f) = light->color;
			}
			break;
		case ActionAlpha:
			if (type == Tween3DTargets)
			{
				auto& tar = targets[a.target.idx]._3d;
				if (tar.alpha)
					(a.dir == ActionForward ? a.v0.f[0] : a.v1.f[0]) = *tar.alpha;
			}
			else if (type == Tween2DTargets)
			{
				auto& tar = targets[a.target.idx]._2d;
				if (tar.alpha)
					(a.dir == ActionForward ? a.v0.f[0] : a.v1.f[0]) = *tar.alpha;
			}
			else if (type == TweenBpCustomRenderer)
				(a.dir == ActionForward ? a.v0.f[0] : a.v1.f[0]) = targets[a.target.idx].renderer_data.alpha;
			break;
		case ActionIntVal:
		{
			auto& tar = targets[a.target.idx];
			if (tar.val.i)
				(a.dir == ActionForward ? a.v0.i[0] : a.v1.i[0]) = *tar.val.i;
		}
			break;
		}
	}

	sTweenPrivate::sTweenPrivate()
	{
	}

	sTweenPrivate::sTweenPrivate(int)
	{
	}

	uint sTweenPrivate::begin()
	{
		auto id = next_id++;
		auto a = new Animation;
		a->type = TweenEntity;
		a->curr_target.e = nullptr;
		a->tracks.emplace_back();
		a->curr_track = a->tracks.begin();
		staging_animations.emplace(id, a);
		return id;
	}

	uint sTweenPrivate::begin_3d_targets()
	{
		auto id = next_id++;
		auto a = new Animation;
		a->type = Tween3DTargets;
		a->curr_target.idx = 0;
		a->tracks.emplace_back();
		a->curr_track = a->tracks.begin();
		staging_animations.emplace(id, a);
		return id;
	}

	uint sTweenPrivate::begin_2d_targets()
	{
		auto id = next_id++;
		auto a = new Animation;
		a->type = Tween2DTargets;
		a->curr_target.idx = 0;
		a->tracks.emplace_back();
		a->curr_track = a->tracks.begin();
		staging_animations.emplace(id, a);
		return id;
	}

	uint sTweenPrivate::begin_bp_custom_renderer(EntityPtr host, BlueprintInstanceGroupPtr renderer, uint targets_count)
	{
		auto id = next_id++;
		auto a = new Animation;
		a->type = TweenBpCustomRenderer;
		a->targets.resize(targets_count);
		a->curr_target.idx = 0;
		a->tracks.emplace_back();
		a->curr_track = a->tracks.begin();
		a->bp_renderer_group = renderer;
		staging_animations.emplace(id, a);

		auto e = Entity::create();
		auto ins = e->add_component<cBpInstance>();
		ins->callbacks.add([a](uint name) {
			if (name == "on_gui"_h)
			{
				auto g = a->bp_renderer_group;
				auto ins = g->instance;
				std::vector<std::pair<uint, voidptr>> inputs;
				inputs.emplace_back(FLAME_HASH_AND_ADDRESS(a->custom_data));
				for (auto i = 0; i < a->targets.size(); i++)
				{
					auto& d = a->targets[i].renderer_data;
					inputs.emplace_back(inputs.size() > 1 ? sh(("pos" + str(i)).c_str()) : "pos"_h, &d.pos);
					inputs.emplace_back(inputs.size() > 1 ? sh(("eul" + str(i)).c_str()) : "eul"_h, &d.eul);
					inputs.emplace_back(inputs.size() > 1 ? sh(("scl" + str(i)).c_str()) : "scl"_h, &d.scl);
					inputs.emplace_back(inputs.size() > 1 ? sh(("alpha" + str(i)).c_str()) : "alpha"_h, &d.alpha);
				}
				ins->call(g, inputs, {});
			}
		});
		a->bp_renderer_entity = e;
		host->add_child(e);

		return id;
	}

	void sTweenPrivate::add_3d_target(uint id, vec3* pos, vec3* eul, vec3* scl, float* alpha)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			auto& t = a->targets.emplace_back();
			t._3d.pos = pos;
			t._3d.eul = eul;
			t._3d.scl = scl;
			t._3d.alpha = alpha;
		}
	}

	void sTweenPrivate::add_2d_target(uint id, vec2* pos, float* ang, vec2* scl, float* alpha)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			auto& t = a->targets.emplace_back();
			t._2d.pos = pos;
			t._2d.ang = ang;
			t._2d.scl = scl;
			t._2d.alpha = alpha;
		}
	}

	void sTweenPrivate::add_int_target(uint id, int* val)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			auto& t = a->targets.emplace_back();
			t.val.i = val;
		}
	}

	void sTweenPrivate::set_target(uint id, EntityPtr e)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			assert(e);
			a->curr_target.e = e;
		}
	}

	void sTweenPrivate::set_target(uint id, uint idx)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			assert(idx < a->targets.size());
			a->curr_target.idx = idx;
		}
	}

	void sTweenPrivate::set_custom_data(uint id, TypeInfo* type, void* data)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			a->custom_data_type = type;
			a->custom_data = type->create();
			type->copy(a->custom_data, data);
		}
	}

	float sTweenPrivate::end(uint id)
	{
		auto total_time = 0.f;
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.release();
			a->time = 0.f;
			for (auto& t : a->tracks)
			{
				if (!t.actions.empty())
					a->action_get_start_value(t.actions.front());
				total_time = max(total_time, t.duration);
			}
			animations.emplace_back(a);
			staging_animations.erase(it);
		}
		return total_time;
	}

	void sTweenPrivate::wait(uint id, float time)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(time);
			a.type = ActionWait;
		}
	}

	void sTweenPrivate::move_to(uint id, const vec3& pos, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionMove;
			a.dir = ActionForward;
			a.v1.f.xyz = pos;
		}
	}

	void sTweenPrivate::move_from(uint id, const vec3& pos, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionMove;
			a.dir = ActionBackward;
			a.v0.f.xyz = pos;
		}
	}

	void sTweenPrivate::rotate_to(uint id, const vec3& eul, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionRotate;
			a.dir = ActionForward;
			a.v1.f.xyz = eul;
		}
	}

	void sTweenPrivate::rotate_from(uint id, const vec3& eul, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionRotate;
			a.dir = ActionBackward;
			a.v0.f.xyz = eul;
		}
	}

	void sTweenPrivate::scale_to(uint id, const vec3& scale, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionScale;
			a.dir = ActionForward;
			a.v1.f.xyz = scale;
		}
	}

	void sTweenPrivate::scale_from(uint id, const vec3& scale, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionScale;
			a.dir = ActionBackward;
			a.v0.f.xyz = scale;
		}
	}

	void sTweenPrivate::object_color_to(uint id, const cvec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionObjectColor;
			a.dir = ActionForward;
			a.v1.c = col;
		}
	}

	void sTweenPrivate::object_color_from(uint id, const cvec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionObjectColor;
			a.dir = ActionBackward;
			a.v0.c = col;
		}
	}

	void sTweenPrivate::light_color_to(uint id, const vec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionLightColor;
			a.dir = ActionForward;
			a.v1.f = col;
		}
	}

	void sTweenPrivate::light_color_from(uint id, const vec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionLightColor;
			a.dir = ActionBackward;
			a.v0.f = col;
		}
	}

	void sTweenPrivate::alpha_to(uint id, float alpha, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionAlpha;
			a.dir = ActionForward;
			a.v1.f[0] = alpha;
		}
	}

	void sTweenPrivate::alpha_from(uint id, float alpha, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionAlpha;
			a.dir = ActionBackward;
			a.v0.f[0] = alpha;
		}
	}

	void sTweenPrivate::int_val_to(uint id, int val, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionIntVal;
			a.dir = ActionForward;
			a.v1.i[0] = val;
		}
	}

	void sTweenPrivate::int_val_from(uint id, int val, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionIntVal;
			a.dir = ActionBackward;
			a.v0.i[0] = val;
		}
	}

	void sTweenPrivate::set_ease(uint id, Ease ease)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& t = *it->second->curr_track;
			if (!t.actions.empty())
				t.actions.back().ease = ease;
		}
	}

	void sTweenPrivate::enable(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionEnable;
		}
	}

	void sTweenPrivate::disable(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionDisable;
		}
	}

	void sTweenPrivate::play_animation(uint id, uint name)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionPlayAnimation;
			a.v1.u[0] = name;
		}
	}

	void sTweenPrivate::kill(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionKill;
		}
	}

	void sTweenPrivate::set_callback(uint id, const std::function<void()>& callback)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& ani = *it->second;
			auto& a = ani.new_action(0.f);
			a.type = ActionCallback;
			a.v1.i[0] = ani.callbacks.size();
			ani.callbacks.push_back(callback);
		}
	}

	void sTweenPrivate::set_callback(uint id, BlueprintInstanceGroupPtr callback)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionBpCallback;
			a.v1.p = callback;
		}
	}

	float sTweenPrivate::get_time(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
			return it->second->curr_track->duration;
		return 0.f;
	}

	void sTweenPrivate::set_channel(uint id, uint ch, float time)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& ani = *it->second;
			if (ani.tracks.size() <= ch)
				ani.tracks.resize(ch + 1);
			ani.curr_track = ani.tracks.begin() + ch;
			auto& t = *ani.curr_track;
			if (t.duration < time)
			{
				auto& a = t.actions.emplace_back();
				a.duration = time - t.duration;
				a.start_time = t.duration;
				a.end_time = a.start_time + a.duration;
				t.duration += a.duration;
			}
		}
	}

	void sTweenPrivate::clear()
	{
		staging_animations.clear();
		animations.clear();
	}

	void sTweenPrivate::update()
	{
		for (auto it = animations.begin(); it != animations.end();)
		{
			auto ani = it->get();
			ani->time += delta_time;

			for (auto it2 = ani->tracks.begin(); it2 != ani->tracks.end(); )
			{
				auto& t = *it2;

				if (!t.actions.empty())
				{
					while (ani->time >= t.actions.front().end_time)
					{
						auto& a = t.actions.front();
						switch (a.type)
						{
						case ActionMove:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_pos(a.v1.f.xyz);
								else if (auto element = a.target.e->get_component<cElement>(); element)
									element->set_pos(a.v1.f.xy);
							}
							else if (ani->type == Tween3DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._3d;
								if (tar.pos)
									*tar.pos = a.v1.f.xyz;
							}
							else if (ani->type == Tween2DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._2d;
								if (tar.pos)
									*tar.pos = a.v1.f.xy;
							}
							else if (ani->type == TweenBpCustomRenderer)
								ani->targets[a.target.idx].renderer_data.pos = a.v1.f.xyz;
							break;
						case ActionRotate:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_eul(a.v1.f.xyz);
								else if (auto element = a.target.e->get_component<cElement>(); element)
									element->set_ang(a.v1.f.x);
							}
							else if (ani->type == Tween3DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._3d;
								if (tar.eul)
									*tar.eul = a.v1.f.xyz;
							}
							else if (ani->type == Tween2DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._2d;
								if (tar.ang)
									*tar.ang = a.v1.f.x;
							}
							else if (ani->type == TweenBpCustomRenderer)
								ani->targets[a.target.idx].renderer_data.eul = a.v1.f.xyz();
							break;
						case ActionScale:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_scl(a.v1.f.xyz);
								else if (auto element = a.target.e->get_component<cElement>(); element)
									element->set_scl(a.v1.f.xy);
							}
							else if (ani->type == Tween3DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._3d;
								if (tar.scl)
									*tar.scl = a.v1.f.xyz;
							}
							else if (ani->type == Tween2DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._2d;
								if (tar.scl)
									*tar.scl = a.v1.f.xy;
							}
							else if (ani->type == TweenBpCustomRenderer)
								ani->targets[a.target.idx].renderer_data.scl = a.v1.f.xyz;
							break;
						case ActionObjectColor:
							if (ani->type == TweenEntity)
							{
								for (auto mesh : a.target.e->get_components<cMesh>(-1))
									mesh->set_color(a.v1.c);
								for (auto mesh : a.target.e->get_components<cSkinnedMesh>(-1))
									mesh->set_color(a.v1.c);
							}
							break;
						case ActionLightColor:
							if (ani->type == TweenEntity)
							{
								if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
									light->set_color(a.v1.f);
								if (auto light = a.target.e->get_component<cPointLight>(); light)
									light->set_color(a.v1.f);
							}
							break;
						case ActionAlpha:
							if (ani->type == Tween3DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._3d;
								if (tar.alpha)
									*tar.alpha = a.v1.f[0];
							}
							else if (ani->type == Tween3DTargets)
							{
								auto& tar = ani->targets[a.target.idx]._2d;
								if (tar.alpha)
									*tar.alpha = a.v1.f[0];
							}
							else if (ani->type == TweenBpCustomRenderer)
								ani->targets[a.target.idx].renderer_data.alpha = a.v1.f[0];
							break;
						case ActionIntVal:
						{
							auto& tar = ani->targets[a.target.idx];
							if (tar.val.i)
								*tar.val.i = a.v1.i[0];
						}
							break;
						case ActionEnable:
							if (ani->type == TweenEntity)
								a.target.e->set_enable(true);
							break;
						case ActionDisable:
							if (ani->type == TweenEntity)
								a.target.e->set_enable(false);
							break;
						case ActionPlayAnimation:
							if (ani->type == TweenEntity)
							{
								if (auto animator = a.target.e->get_component<cAnimator>(); animator)
									animator->play(a.v1.u[0]);
							}
							break;
						case ActionKill:
							if (ani->type == TweenEntity)
								sGraveyard::instance()->add(a.target.e);
							break;
						case ActionCallback:
						{
							auto idx = a.v1.i[0];
							ani->callbacks[idx]();
						}
							break;
						case ActionBpCallback:
						{
							auto g = (BlueprintInstanceGroupPtr)a.v1.p;
							auto bp_ins = g->instance;
							voidptr inputs[1];
							inputs[0] = ani->custom_data;
							bp_ins->call(g, inputs, nullptr);
						}
							break;
						}

						t.actions.pop_front();
						if (t.actions.empty())
							break;
						ani->action_get_start_value(t.actions.front());
					}
				}

				if (!t.actions.empty())
				{
					auto& a = t.actions.front();
					auto t = (ani->time - a.start_time) / a.duration;
					switch (a.ease)
					{
					case EaseInSine:
						t = sineEaseIn(t);
						break;
					case EaseOutSine:
						t = sineEaseOut(t);
						break;
					case EaseInOutSine:
						t = sineEaseInOut(t);
						break;
					case EaseInQuad:
						t = quadraticEaseIn(t);
						break;
					case EaseOutQuad:
						t = quadraticEaseOut(t);
						break;
					case EaseInOutQuad:
						t = quadraticEaseInOut(t);
						break;
					case EaseInCubic:
						t = cubicEaseIn(t);
						break;
					case EaseOutCubic:
						t = cubicEaseOut(t);
						break;
					case EaseInOutCubic:
						t = cubicEaseInOut(t);
						break;
					case EaseInQuart:
						t = quarticEaseIn(t);
						break;
					case EaseOutQuart:
						t = quarticEaseOut(t);
						break;
					case EaseInOutQuart:
						t = quarticEaseInOut(t);
						break;
					case EaseInQuint:
						t = quinticEaseIn(t);
						break;
					case EaseOutQuint:
						t = quinticEaseOut(t);
						break;
					case EaseInOutQuint:
						t = quinticEaseInOut(t);
						break;
					case EaseInExpo:
						t = exponentialEaseIn(t);
						break;
					case EaseOutExpo:
						t = exponentialEaseOut(t);
						break;
					case EaseInOutExpo:
						t = exponentialEaseInOut(t);
						break;
					case EaseInCirc:
						t = circularEaseIn(t);
						break;
					case EaseOutCirc:
						t = circularEaseOut(t);
						break;
					case EaseInOutCirc:
						t = circularEaseInOut(t);
						break;
					case EaseInBack:
						t = backEaseIn(t);
						break;
					case EaseOutBack:
						t = backEaseOut(t);
						break;
					case EaseInOutBack:
						t = backEaseInOut(t);
						break;
					case EaseInElastic:
						t = elasticEaseIn(t);
						break;
					case EaseOutElastic:
						t = elasticEaseOut(t);
						break;
					case EaseInOutElastic:
						t = elasticEaseInOut(t);
						break;
					case EaseInBounce:
						t = bounceEaseIn(t);
						break;
					case EaseOutBounce:
						t = bounceEaseOut(t);
						break;
					case EaseInOutBounce:
						t = bounceEaseInOut(t);
						break;
					}
					switch (a.type)
					{
					case ActionMove:
						if (ani->type == TweenEntity)
						{
							if (auto node = a.target.e->get_component<cNode>(); node)
								node->set_pos(mix(a.v0.f.xyz(), a.v1.f.xyz(), t));
							else if (auto element = a.target.e->get_component<cElement>(); element)
								element->set_pos(mix(a.v0.f.xy(), a.v1.f.xy(), t));
						}
						else if (ani->type == Tween3DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._3d;
							if (tar.pos)
								*tar.pos = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						}
						else if (ani->type == Tween2DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._2d;
							if (tar.pos)
								*tar.pos = mix(a.v0.f.xy(), a.v1.f.xy(), t);
						}
						else if (ani->type == TweenBpCustomRenderer)
							ani->targets[a.target.idx].renderer_data.pos = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						break;
					case ActionRotate:
						if (ani->type == TweenEntity)
						{
							if (auto node = a.target.e->get_component<cNode>(); node)
								node->set_eul(mix(a.v0.f.xyz(), a.v1.f.xyz(), t));
							else if (auto element = a.target.e->get_component<cElement>(); element)
								element->set_ang(mix(a.v0.f.x, a.v1.f.x, t));
						}
						else if (ani->type == Tween3DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._3d;
							if (tar.eul)
								*tar.eul = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						}
						else if (ani->type == Tween2DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._2d;
							if (tar.ang)
								*tar.ang = mix(a.v0.f.x, a.v1.f.x, t);
						}
						else if (ani->type == TweenBpCustomRenderer)
							ani->targets[a.target.idx].renderer_data.eul = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						break;
					case ActionScale:
						if (ani->type == TweenEntity)
						{
							if (auto node = a.target.e->get_component<cNode>(); node)
								node->set_scl(mix(a.v0.f.xyz(), a.v1.f.xyz(), t));
							else if (auto element = a.target.e->get_component<cElement>(); element)
								element->set_scl(mix(a.v0.f.xy(), a.v1.f.xy(), t));
						}
						else if (ani->type == Tween3DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._3d;
							if (tar.scl)
								*tar.scl = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						}
						else if (ani->type == Tween2DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._2d;
							if (tar.scl)
								*tar.scl = mix(a.v0.f.xy(), a.v1.f.xy(), t);
						}
						else if (ani->type == TweenBpCustomRenderer)
							ani->targets[a.target.idx].renderer_data.scl = mix(a.v0.f.xyz(), a.v1.f.xyz(), t);
						break;
					case ActionObjectColor:
						if (ani->type == TweenEntity)
						{
							auto col = mix(a.v0.c, a.v1.c, t);
							for (auto mesh : a.target.e->get_components<cMesh>(-1))
								mesh->set_color(col);
							for (auto mesh : a.target.e->get_components<cSkinnedMesh>(-1))
								mesh->set_color(col);
						}
						break;
					case ActionLightColor:
						if (ani->type == TweenEntity)
						{
							auto col = mix(a.v0.f, a.v1.f, t);
							if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
								light->set_color(col);
							if (auto light = a.target.e->get_component<cPointLight>(); light)
								light->set_color(col);
						}
						break;
					case ActionAlpha:
						if (ani->type == Tween3DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._3d;
							if (tar.alpha)
								*tar.alpha = mix(a.v0.f[0], a.v1.f[0], t);
						}
						else if (ani->type == Tween2DTargets)
						{
							auto& tar = ani->targets[a.target.idx]._2d;
							if (tar.alpha)
								*tar.alpha = mix(a.v0.f[0], a.v1.f[0], t);
						}
						else if (ani->type == TweenBpCustomRenderer)
							ani->targets[a.target.idx].renderer_data.alpha = mix(a.v0.f[0], a.v1.f[0], t);
						break;
					case ActionIntVal:
					{
						auto& tar = ani->targets[a.target.idx];
						if (tar.val.i)
							*tar.val.i = mix(a.v0.i[0], a.v1.i[0], t);
					}
						break;
					}
				}

				if (t.actions.empty())
					it2 = ani->tracks.erase(it2);
				else
					it2++;
			}

			if (ani->tracks.empty())
				it = animations.erase(it);
			else
				it++;
		}
	}

	static sTweenPtr _instance = nullptr;

	struct sTweenInstance : sTween::Instance
	{
		sTweenPtr operator()() override
		{
			return _instance;
		}
	}sTween_instance;
	sTween::Instance& sTween::instance = sTween_instance;

	struct sTweenCreate : sTween::Create
	{
		sTweenPtr operator()(WorldPtr w) override
		{
			if (!w)
				return new sTweenPrivate(0);

			assert(!_instance);
			_instance = new sTweenPrivate();
			return _instance;
		}
	}sTween_create;
	sTween::Create& sTween::create = sTween_create;
}

