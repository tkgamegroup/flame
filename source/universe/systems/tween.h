#pragma once

#include "../system.h"

namespace flame
{
	enum TweenType
	{
		TweenEntity,
		Tween3DTargets,
		Tween2DTargets,
		TweenBpCustomRenderer
	};

	enum Ease
	{
		EaseLinear,
		EaseInSine,
		EaseOutSine,
		EaseInOutSine,
		EaseInQuad,
		EaseOutQuad,
		EaseInOutQuad,
		EaseInCubic,
		EaseOutCubic,
		EaseInOutCubic,
		EaseInQuart,
		EaseOutQuart,
		EaseInOutQuart,
		EaseInQuint,
		EaseOutQuint,
		EaseInOutQuint,
		EaseInExpo,
		EaseOutExpo,
		EaseInOutExpo,
		EaseInCirc,
		EaseOutCirc,
		EaseInOutCirc,
		EaseInBack,
		EaseOutBack,
		EaseInOutBack,
		EaseInElastic,
		EaseOutElastic,
		EaseInOutElastic,
		EaseInBounce,
		EaseOutBounce,
		EaseInOutBounce
	};

	struct sTween : System
	{
		virtual uint begin() = 0;
		virtual uint begin_3d_targets(uint targets_count) = 0;
		virtual uint begin_2d_targets(uint targets_count) = 0;
		virtual uint begin_bp_custom_renderer(EntityPtr host, BlueprintInstanceGroupPtr renderer, uint targets_count) = 0;
		virtual void setup_3d_target(uint id, uint idx, vec3* pos, vec3* eul, vec3* scl, float* alpha) = 0;
		virtual void setup_2d_target(uint id, uint idx, vec2* pos, float* ang, vec2* scl, float* alpha) = 0;
		virtual void set_target(uint id, EntityPtr e) = 0;
		virtual void set_target(uint id, uint idx) = 0;
		virtual void set_custom_data(uint id, TypeInfo* type, void* data) = 0;
		virtual void end(uint id) = 0;
		virtual void wait(uint id, float time) = 0;
		virtual void move_to(uint id, const vec3& pos, float duration) = 0;
		inline void move_to(uint id, const vec2& pos, float duration)
		{
			move_to(id, vec3(pos, 0.f), duration);
		}
		virtual void move_from(uint id, const vec3& pos, float duration) = 0;
		inline void move_from(uint id, const vec2& pos, float duration)
		{
			move_from(id, vec3(pos, 0.f), duration);
		}
		virtual void rotate_to(uint id, const vec3& eul, float duration) = 0;
		virtual void rotate_from(uint id, const vec3& eul, float duration) = 0;
		virtual void scale_to(uint id, const vec3& scl, float duration) = 0;
		inline void scale_to(uint id, const vec2& scl, float duration)
		{
			scale_to(id, vec3(scl, 0.f), duration);
		}
		virtual void scale_from(uint id, const vec3& scl, float duration) = 0;
		inline void scale_from(uint id, const vec2& scl, float duration)
		{
			scale_from(id, vec3(scl, 0.f), duration);
		}
		virtual void object_color_to(uint id, const cvec4& col, float duration) = 0;
		virtual void object_color_from(uint id, const cvec4& col, float duration) = 0;
		virtual void light_color_to(uint id, const vec4& col, float duration) = 0;
		virtual void light_color_from(uint id, const vec4& col, float duration) = 0;
		virtual void alpha_to(uint id, float alpha, float duration) = 0;
		virtual void alpha_from(uint id, float alpha, float duration) = 0;
		virtual void set_ease(uint id, Ease ease) = 0;
		virtual void enable(uint id) = 0;
		virtual void disable(uint id) = 0;
		virtual void play_animation(uint id, uint name) = 0;
		virtual void kill(uint id) = 0;
		virtual void set_callback(uint id, const std::function<void()>& callback) = 0;
		virtual void set_callback(uint id, BlueprintInstanceGroupPtr callback) = 0;

		virtual void set_channel(uint id, uint ch, bool sync_last_action = true, bool sync_to_begin = true) = 0;

		virtual void clear() = 0;

		struct Instance
		{
			virtual sTweenPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual sTweenPtr operator()(WorldPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
