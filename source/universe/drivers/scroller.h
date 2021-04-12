#pragma once

#include "../driver.h"

namespace flame
{
	struct dScroller : Driver
	{
		inline static auto type_name = "flame::dScroller";
		inline static auto type_hash = ch(type_name);

		dScroller() :
			Driver(type_name, type_hash)
		{
		}

		virtual ScrollType get_type() const = 0;
		virtual void set_type(ScrollType type) = 0;

		virtual void scroll(const vec2& v) = 0;

		FLAME_UNIVERSE_EXPORTS static dScroller* create(void* parms = nullptr);
	};
}
