#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cElement : Component
	{
		// Reflect
		StaticState static_state = NotStatic;

		// Reflect
		vec2 pos = vec2(0.f);
		// Reflect
		virtual void set_pos(const vec2& pos) = 0;
		inline void add_pos(const vec2& p)
		{
			set_pos(pos + p);
		}

		// Reflect
		vec2 ext = vec2(1.f);
		// Reflect
		virtual void set_ext(const vec2& ext) = 0;
		inline void add_ext(const vec2& e)
		{
			set_ext(ext + e);
		}

		// Reflect
		vec2 scl = vec2(1.f);
		// Reflect
		virtual void set_scl(const vec2& s) = 0;
		inline void add_scl(const vec2& s)
		{
			set_scl(scl + s);
		}

		// Reflect
		cvec4 background_col = cvec4(0);
		// Reflect
		virtual void set_background_col(const cvec4& col) = 0;

		// Reflect
		cvec4 frame_col = cvec4(0);
		// Reflect
		virtual void set_frame_col(const cvec4& col) = 0;

		// Reflect
		float frame_thickness = 1.f;
		// Reflect
		virtual void set_frame_thickness(float thickness) = 0;

		// Reflect
		bool scissor = false;
		// Reflect
		virtual void set_scissor(bool v) = 0;

		// Reflect
		ElementAlign align = ElementAlignNone;
		// Reflect
		virtual void set_align(ElementAlign align) = 0;

		mat3x2 transform;
		vec2 global_pos0() { return transform[2]; }
		vec2 global_pos1() { return transform * vec3(ext, 1.f); }

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;

		Listeners<void(graphics::CanvasPtr canvas)> drawers;

		virtual bool update_transform() = 0;

		struct Create
		{
			virtual cElementPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
