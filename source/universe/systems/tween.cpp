#include "../../foundation/blueprint.h"
#include "../entity_private.h"
#include "../components/node_private.h"
#include "../components/mesh_private.h"
#include "../components/skinned_mesh_private.h"
#include "../components/animator_private.h"
#include "../components/directional_light_private.h"
#include "../components/point_light_private.h"
#include "tween_private.h"
#include "graveyard_private.h"
#include "renderer_private.h"

namespace flame
{
	sTweenPrivate::Animation::~Animation()
	{
		if (type == TweenGui)
			sRenderer::instance()->hud_callbacks.remove((uint)this);
		if (custom_data)
			custom_data_type->destroy(custom_data);
	}

	sTweenPrivate::Action& sTweenPrivate::Animation::new_action(float duration)
	{
		if (newline)
		{
			auto& last_t = *curr_track;
			auto start_time = last_t.actions.empty() ? 0.f : last_t.actions.back().start_time;

			curr_track++;
			if (curr_track == tracks.end())
			{
				tracks.emplace_back();
				curr_track = tracks.end();
				curr_track--;
			}
			newline = false;

			auto& t = *curr_track;
			if (t.duration < start_time)
			{
				auto& a = t.actions.emplace_back();
				a.duration = start_time - t.duration;
				a.start_time = t.duration;
				a.end_time = a.start_time + a.duration;
				t.duration += a.duration;
			}
		}
		else
		{
			curr_track = tracks.begin();
			auto& t = *curr_track;

			auto max_duration = 0.f;
			for (auto& t : tracks)
				max_duration = std::max(max_duration, t.duration);

			if (t.duration < max_duration)
			{
				auto& a = t.actions.emplace_back();
				a.duration = max_duration - t.duration;
				a.start_time = t.duration;
				a.end_time = a.start_time + a.duration;
				t.duration += a.duration;
			}
		}

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
		case ActionMoveTo:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					a.v0.f.xyz = node->pos;
			}
			else if (type == TweenGui)
				a.v0.f.xyz = renderer_datas[a.target.idx].pos;
			break;
		case ActionRotateTo:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					a.v0.f.xyz = node->get_eul();
			}
			else if (type == TweenGui)
				a.v0.f.xyz =renderer_datas[a.target.idx].eul;
			break;
		case ActionScaleTo:
			if (type == TweenEntity)
			{
				if (auto node = a.target.e->get_component<cNode>(); node)
					a.v0.f.xyz = node->scl;
			}
			else if (type == TweenGui)
				a.v0.f.xyz =renderer_datas[a.target.idx].scl;
			break;
		case ActionObjectColorTo:
			if (type == TweenEntity)
			{
				auto meshes = a.target.e->get_components<cMesh>(-1);
				if (!meshes.empty())
					a.v0.c = meshes[0]->color;
				else
				{
					auto skinned_meshes = a.target.e->get_components<cSkinnedMesh>(-1);
					if (!skinned_meshes.empty())
						a.v0.c = skinned_meshes[0]->color;
				}
			}
			break;
		case ActionLightColorTo:
			if (type == TweenEntity)
			{
				if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
					a.v0.f = light->color;
				if (auto light = a.target.e->get_component<cPointLight>(); light)
					a.v0.f = light->color;
			}
			break;
		case ActionAlphaTo:
			if (type == TweenGui)
				a.v0.f[0] =renderer_datas[a.target.idx].alpha;
			break;
		}
	}

	sTweenPrivate::sTweenPrivate()
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

	uint sTweenPrivate::begin(BlueprintInstanceGroupPtr renderer, uint target_count)
	{
		auto id = next_id++;
		auto a = new Animation;
		a->type = TweenGui;
		a->curr_target.idx = 0;
		a->tracks.emplace_back();
		a->curr_track = a->tracks.begin();
		a->renderer = renderer;
		a->renderer_datas.resize(target_count);
		staging_animations.emplace(id, a);
		return id;
	}

	void sTweenPrivate::set_target(uint id, EntityPtr e)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
			a->curr_target.e = e;
		}
	}

	void sTweenPrivate::set_target(uint id, uint idx)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.get();
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

	void sTweenPrivate::end(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto a = it->second.release();
			a->time = 0.f;
			if (a->type == TweenGui)
			{
				sRenderer::instance()->hud_callbacks.add([a]() {
					auto g = a->renderer;
					auto ins = g->instance;
					std::vector<std::pair<uint, voidptr>> inputs;
					inputs.emplace_back(FLAME_HASH_AND_ADDRESS(a->custom_data));
					for (auto i = 0; i < a->renderer_datas.size(); i++)
					{
						auto& d = a->renderer_datas[i];
						inputs.emplace_back(inputs.size() > 1 ? sh(("pos" + str(i)).c_str()) : "pos"_h, &d.pos);
						inputs.emplace_back(inputs.size() > 1 ? sh(("eul" + str(i)).c_str()) : "eul"_h, &d.eul);
						inputs.emplace_back(inputs.size() > 1 ? sh(("scl" + str(i)).c_str()) : "scl"_h, &d.scl);
						inputs.emplace_back(inputs.size() > 1 ? sh(("alpha" + str(i)).c_str()) : "alpha"_h, &d.alpha);
					}
					ins->call(g, inputs, {});
				}, (uint)a);
			}
			for (auto& t : a->tracks)
			{
				if (!t.actions.empty())
					a->action_get_start_value(t.actions.front());
			}
			animations.emplace_back(a);
			staging_animations.erase(it);
		}
	}

	void sTweenPrivate::newline(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
			it->second->newline = true;
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
			a.type = ActionMoveTo;
			a.v1.f.xyz = pos;
		}
	}

	void sTweenPrivate::rotate_to(uint id, const vec3& eul, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionRotateTo;
			a.v1.f.xyz = eul;
		}
	}

	void sTweenPrivate::scale_to(uint id, const vec3& scale, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionScaleTo;
			a.v1.f.xyz = scale;
		}
	}

	void sTweenPrivate::object_color_to(uint id, const cvec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionObjectColorTo;
			a.v1.c = col;
		}
	}

	void sTweenPrivate::light_color_to(uint id, const vec4& col, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionLightColorTo;
			a.v1.f = col;
		}
	}

	void sTweenPrivate::alpha_to(uint id, float alpha, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(duration);
			a.type = ActionAlphaTo;
			a.v1.f[0] = alpha;
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

	void sTweenPrivate::set_callback(uint id, BlueprintInstanceGroupPtr callback)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second->new_action(0.f);
			a.type = ActionCallback;
			a.v1.p = callback;
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
						case ActionMoveTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_pos(a.v1.f.xyz);
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].pos = a.v1.f.xyz;
							break;
						case ActionRotateTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_eul(a.v1.f.xyz());
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].eul = a.v1.f.xyz();
							break;
						case ActionScaleTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_scl(a.v1.f.xyz);
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].scl = a.v1.f.xyz;
							break;
						case ActionObjectColorTo:
							if (ani->type == TweenEntity)
							{
								for (auto mesh : a.target.e->get_components<cMesh>(-1))
									mesh->set_color(a.v1.c);
								for (auto mesh : a.target.e->get_components<cSkinnedMesh>(-1))
									mesh->set_color(a.v1.c);
							}
							break;
						case ActionLightColorTo:
							if (ani->type == TweenEntity)
							{
								if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
									light->color = a.v1.f;
								if (auto light = a.target.e->get_component<cPointLight>(); light)
									light->color = a.v1.f;
							}
							break;
						case ActionAlphaTo:
							if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].alpha = a.v1.f[0];
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
					{
						switch (a.type)
						{
						case ActionMoveTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_pos(mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration));
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].pos = mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration);
							break;
						case ActionRotateTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_eul(mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration));
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].eul = mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration);
							break;
						case ActionScaleTo:
							if (ani->type == TweenEntity)
							{
								if (auto node = a.target.e->get_component<cNode>(); node)
									node->set_scl(mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration));
							}
							else if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].scl = mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani->time - a.start_time) / a.duration);
							break;
						case ActionObjectColorTo:
							if (ani->type == TweenEntity)
							{
								auto col = mix(a.v0.c, a.v1.c, (ani->time - a.start_time) / a.duration);
								for (auto mesh : a.target.e->get_components<cMesh>(-1))
									mesh->set_color(col);
								for (auto mesh : a.target.e->get_components<cSkinnedMesh>(-1))
									mesh->set_color(col);
							}
							break;
						case ActionLightColorTo:
							if (ani->type == TweenEntity)
							{
								auto col = mix(a.v0.f, a.v1.f, (ani->time - a.start_time) / a.duration);
								if (auto light = a.target.e->get_component<cDirectionalLight>(); light)
									light->color = col;
								if (auto light = a.target.e->get_component<cPointLight>(); light)
									light->color = col;
							}
							break;
						case ActionAlphaTo:
							if (ani->type == TweenGui)
								ani->renderer_datas[a.target.idx].alpha = mix(a.v0.f[0], a.v1.f[0], (ani->time - a.start_time) / a.duration);
							break;
						}
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
			if (_instance == nullptr)
				_instance = new sTweenPrivate();
			return _instance;
		}
	}sTween_instance;
	sTween::Instance& sTween::instance = sTween_instance;

	struct sTweenCreate : sTween::Create
	{
		sTweenPtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sTweenPrivate();
			return _instance;
		}
	}sTween_create;
	sTween::Create& sTween::create = sTween_create;
}

