#pragma once

#include "../component.h"

namespace flame
{
	struct cScroller : Component
	{
		inline static auto type_name = "flame::cScroller";
		inline static auto type_hash = ch(type_name);

		cScroller() : Component(type_name, type_hash)
		{
		}

		virtual ScrollType get_type() const = 0;
		virtual void set_type(ScrollType type) = 0;

		virtual void scroll(const vec2& v) = 0;

		FLAME_UNIVERSE_EXPORTS static cScroller* create(void* parms = nullptr);
	};
}
