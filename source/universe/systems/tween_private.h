#pragma once

#include "tween.h"

namespace flame
{
	struct sTweenPrivate : sTween
	{
		struct RendererData
		{
			vec3  pos = vec3(0.f);
			vec3  eul = vec3(0.f);
			vec3  scl = vec3(1.f);
			float alpha = 1.f;
		};

		union Target
		{
			EntityPtr e;
			uint idx;
		};

		enum ActionType
		{
			ActionWait,
			ActionMove,
			ActionRotate,
			ActionScale,
			ActionObjectColor,
			ActionLightColor,
			ActionAlpha,
			ActionEnable,
			ActionDisable,
			ActionPlayAnimation,
			ActionKill,
			ActionCallback,
			ActionBpCallback
		};

		enum ActionDirection
		{
			ActionForward,
			ActionBackward
		};

		struct Action
		{
			Target target;

			float start_time;
			float end_time;
			ActionType type;
			ActionDirection dir;
			Ease ease = EaseLinear;
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
			TweenType type;
			EntityPtr renderer_entity = nullptr;
			BlueprintInstanceGroupPtr renderer = nullptr;
			std::vector<RendererData> renderer_datas;
			vec3* pos_data = nullptr; 
			vec3* eul_data = nullptr; 
			vec3* scl_data = nullptr; 
			float* alpha_data = nullptr;
			Target curr_target;

			std::vector<std::function<void()>> callbacks;
			TypeInfo*	custom_data_type = nullptr;
			void*		custom_data = nullptr;
			std::list<Track> tracks;

			// building
			std::list<Track>::iterator curr_track;
			bool newline = false;

			// running
			float time;

			~Animation();
			Action& new_action(float duration);
			void action_get_start_value(Action& a);
		};

		uint next_id = 1;

		std::unordered_map<uint, std::unique_ptr<Animation>> staging_animations;
		std::vector<std::unique_ptr<Animation>> animations;

		sTweenPrivate();

		uint begin() override;
		uint begin(vec3* pos_data, vec3* eul_data, vec3* scl_data, float* alpha_data) override;
		uint begin(EntityPtr host, BlueprintInstanceGroupPtr renderer, uint target_count) override;
		void set_target(uint id, EntityPtr e) override;
		void set_target(uint id, uint idx) override;
		void set_custom_data(uint id, TypeInfo* type, void* data) override;
		void end(uint id) override;
		void newline(uint id) override;
		void wait(uint id, float time) override;
		void move_to(uint id, const vec3& pos, float duration) override;
		void move_from(uint id, const vec3& pos, float duration) override;
		void rotate_to(uint id, const vec3& eul, float duration) override;
		void rotate_from(uint id, const vec3& eul, float duration) override;
		void scale_to(uint id, const vec3& scale, float duration) override;
		void scale_from(uint id, const vec3& scale, float duration) override;
		void object_color_to(uint id, const cvec4& col, float duration) override;
		void object_color_from(uint id, const cvec4& col, float duration) override;
		void light_color_to(uint id, const vec4& col, float duration) override;
		void light_color_from(uint id, const vec4& col, float duration) override;
		void alpha_to(uint id, float alpha, float duration) override;
		void alpha_from(uint id, float alpha, float duration) override;
		void set_ease(uint id, Ease ease) override;
		void enable(uint id) override;
		void disable(uint id) override;
		void play_animation(uint id, uint name) override;
		void kill(uint id) override;
		void set_callback(uint id, const std::function<void()>& callback) override;
		void set_callback(uint id, BlueprintInstanceGroupPtr callback) override;

		void clear() override;

		void update() override;
	};
}
