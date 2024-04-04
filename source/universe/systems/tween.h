#pragma once

#include "../system.h"

namespace flame
{
	enum TweenType
	{
		TweenEntity,
		TweenGui
	};

	struct sTween : System
	{
		virtual uint begin() = 0;
		virtual uint begin(EntityPtr renderer_parent, BlueprintInstanceGroupPtr renderer, uint target_count) = 0;
		virtual void set_target(uint id, EntityPtr e) = 0;
		virtual void set_target(uint id, uint idx) = 0;
		virtual void set_custom_data(uint id, TypeInfo* type, void* data) = 0;
		virtual void end(uint id) = 0;
		virtual void newline(uint id) = 0;
		virtual void wait(uint id, float time) = 0;
		virtual void move_to(uint id, const vec3& pos, float duration) = 0;
		virtual void move_from(uint id, const vec3& pos, float duration) = 0;
		virtual void rotate_to(uint id, const vec3& eul, float duration) = 0;
		virtual void rotate_from(uint id, const vec3& eul, float duration) = 0;
		virtual void scale_to(uint id, const vec3& scl, float duration) = 0;
		virtual void scale_from(uint id, const vec3& scl, float duration) = 0;
		virtual void object_color_to(uint id, const cvec4& col, float duration) = 0;
		virtual void object_color_from(uint id, const cvec4& col, float duration) = 0;
		virtual void light_color_to(uint id, const vec4& col, float duration) = 0;
		virtual void light_color_from(uint id, const vec4& col, float duration) = 0;
		virtual void alpha_to(uint id, float alpha, float duration) = 0;
		virtual void alpha_from(uint id, float alpha, float duration) = 0;
		virtual void enable(uint id) = 0;
		virtual void disable(uint id) = 0;
		virtual void play_animation(uint id, uint name) = 0;
		virtual void kill(uint id) = 0;
		virtual void set_callback(uint id, BlueprintInstanceGroupPtr callback) = 0;

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
