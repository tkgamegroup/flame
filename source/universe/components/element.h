#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cElement : Component
	{
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

		mat2 transform;

		virtual void mark_transform_dirty() = 0;
		virtual void mark_drawing_dirty() = 0;

		struct Create
		{
			virtual cElementPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
