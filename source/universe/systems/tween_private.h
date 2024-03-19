#pragma once

#include "tween.h"

namespace flame
{
	struct sTweenPrivate : sTween
	{
		enum ActionType
		{
			ActionWait,
			ActionMoveTo,
			ActionRotateTo,
			ActionScaleTo,
			ActionPlayAnimation,
			ActionKill
		};

		struct Action
		{
			float start_time;
			float end_time;
			ActionType type;
			float duration;
			lVariant v0, v1;
		};

		struct Track
		{
			float duration = 0.f;
			std::list<Action> actions;
		};

		struct Animation
		{
			EntityPtr target = nullptr;
			RendererType renderer_type;
			BlueprintInstanceGroupPtr bp_renderer = nullptr;
			vec3 renderer_pos;
			quat renderer_qut;
			vec3 renderer_scl;
			std::list<Track> tracks;

			// building
			std::list<Track>::iterator curr_track;
			bool newline = false;

			// running
			float time;

			Action& new_action(float duration);
		};

		uint next_id = 1;

		std::unordered_map<uint, Animation> staging_animations;
		std::vector<std::unique_ptr<Animation>> animations;

		sTweenPrivate();

		uint begin(EntityPtr e) override;
		uint begin(RendererType render_type, BlueprintInstanceGroupPtr render) override;
		void end(uint id) override;
		void newline(uint id) override;
		void wait(uint id, float time) override;
		void move_to(uint id, const vec3& pos, float duration) override;
		void rotate_to(uint id, const quat& qut, float duration) override;
		void scale_to(uint id, const vec3& scale, float duration) override;
		void play_animation(uint id, uint name) override;
		void kill(uint id) override;

		void update() override;
	};
}
