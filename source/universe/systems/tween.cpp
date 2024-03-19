#include "../entity_private.h"
#include "../components/node_private.h"
#include "../components/animator_private.h"
#include "tween_private.h"
#include "graveyard_private.h"
#include "renderer_private.h"

namespace flame
{
	sTweenPrivate::Action& sTweenPrivate::Animation::new_action(float duration)
	{
		auto max_duration = 0.f;
		for (auto& t : tracks)
			max_duration = std::max(max_duration, t.duration);

		auto get_track = [&]() -> Track& {
			if (newline)
			{
				curr_track++;
				if (curr_track == tracks.end())
				{
					tracks.emplace_back();
					curr_track = tracks.end();
					curr_track--;
				}
				newline = false;

				return *curr_track;
			}

			curr_track = tracks.begin();
			return tracks.front();
		};

		auto& t = get_track();
		if (t.duration < max_duration)
		{
			auto& a = t.actions.emplace_back();
			a.duration = max_duration - t.duration;
			a.start_time = t.duration;
			a.end_time = a.start_time + a.duration;
			t.duration += a.duration;
		}
		auto& a = t.actions.emplace_back();
		a.duration = duration;
		a.start_time = t.duration;
		a.end_time = a.start_time + duration;
		t.duration += duration;
		return a;
	}

	sTweenPrivate::sTweenPrivate()
	{
	}

	uint sTweenPrivate::begin(EntityPtr e)
	{
		auto id = next_id++;
		auto& a = staging_animations[id];
		a.target = e;
		a.tracks.emplace_back();
		a.curr_track = a.tracks.begin();
		return id;
	}

	uint sTweenPrivate::begin(RendererType render_type, BlueprintInstanceGroupPtr render)
	{
		auto id = next_id++;
		auto& a = staging_animations[id];
		a.renderer_type = render_type;
		a.bp_renderer = render;
		a.tracks.emplace_back();

		a.renderer_pos = vec3(0.f);
		a.renderer_qut = quat(1.f, 0.f, 0.f, 0.f);
		a.renderer_scl = vec3(1.f);

		return id;
	}

	void sTweenPrivate::end(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			it->second.time = 0.f;

			auto a = new Animation(std::move(it->second));
			if (it->second.renderer_type == RendererGui)
			{
				sRenderer::instance()->hud_callbacks.add([a]() {

				}, (uint)a);
			}
			animations.emplace_back(a);
			staging_animations.erase(it);
		}
	}

	void sTweenPrivate::newline(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
			it->second.newline = true;
	}

	void sTweenPrivate::wait(uint id, float time)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(time);
			a.type = ActionWait;
		}
	}

	void sTweenPrivate::move_to(uint id, const vec3& pos, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(duration);
			a.type = ActionMoveTo;
			a.v0.f.xyz = pos;
		}
	}

	void sTweenPrivate::rotate_to(uint id, const quat& qut, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(duration);
			a.type = ActionRotateTo;
			a.v0.f = vec4(qut.x, qut.y, qut.z, qut.w);
		}
	}

	void sTweenPrivate::scale_to(uint id, const vec3& scale, float duration)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(duration);
			a.type = ActionScaleTo;
			a.v0.f.xyz = scale;
		}
	}

	void sTweenPrivate::play_animation(uint id, uint name)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(0.f);
			a.type = ActionPlayAnimation;
			a.v0.u[0] = name;
		}
	}

	void sTweenPrivate::kill(uint id)
	{
		if (auto it = staging_animations.find(id); it != staging_animations.end())
		{
			auto& a = it->second.new_action(0.f);
			a.type = ActionKill;
		}
	}

	void sTweenPrivate::update()
	{
		for (auto it = animations.begin(); it != animations.end();)
		{
			auto& ani = *it->get();
			ani.time += delta_time;

			for (auto it2 = ani.tracks.begin(); it2 != ani.tracks.end(); )
			{
				auto& t = *it2;

				if (!t.actions.empty())
				{
					while (ani.time >= t.actions.front().end_time)
					{
						auto& a = t.actions.front();
						switch (a.type)
						{
						case ActionMoveTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									node->set_pos(a.v1.f.xyz);
							}
							break;
						case ActionRotateTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
								{
									auto& v = a.v1.f;
									node->set_qut(quat(v.w, v.x, v.y, v.z));
								}
							}
							break;
						case ActionScaleTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									node->set_scl(a.v1.f.xyz);
							}
							break;
						case ActionPlayAnimation:
							if (ani.target)
							{
								if (auto animator = ani.target->get_component<cAnimator>(); animator)
									animator->play(a.v0.u[0]);
							}
							break;
						case ActionKill:
							if (ani.target)
								sGraveyard::instance()->add(ani.target);
							break;
						}

						t.actions.pop_front();
						if (t.actions.empty())
							break;
						a = t.actions.front();
						switch (a.type)
						{
						case ActionMoveTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									a.v0.f.xyz = node->pos;
							}
							break;
						case ActionRotateTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
								{
									auto& q = node->qut;
									a.v0.f = vec4(q.x, q.y, q.z, q.w);
								}
							}
							break;
						case ActionScaleTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									a.v1.f.xyz = node->scl;
							}
							break;
						}
					}
				}

				if (!t.actions.empty())
				{
					auto& a = t.actions.front();
					{
						switch (a.type)
						{
						case ActionMoveTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									node->set_pos(mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani.time - a.start_time) / a.duration));
							}
							break;
						case ActionRotateTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
								{
									auto& v0 = a.v0.f; auto& v1 = a.v1.f;
									node->set_qut(slerp(quat(v0.w, v0.x, v0.y, v0.z), quat(v1.w, v1.x, v1.y, v1.z), (ani.time - a.start_time) / a.duration));
								}
							}
							break;
						case ActionScaleTo:
							if (ani.target)
							{
								if (auto node = ani.target->get_component<cNode>(); node)
									node->set_scl(mix(a.v0.f.xyz(), a.v1.f.xyz(), (ani.time - a.start_time) / a.duration));
							}
							break;
						}
					}
				}

				if (t.actions.empty())
					it2 = ani.tracks.erase(it2);
				else
					it2++;
			}

			if (ani.tracks.empty())
			{
				if (ani.renderer_type == RendererGui)
					sRenderer::instance()->hud_callbacks.remove((uint)&ani);
				it = animations.erase(it);
			}
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

