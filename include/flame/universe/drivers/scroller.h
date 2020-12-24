#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct dScroller : Component
	{
		inline static auto type_name = "flame::dScroller";
		inline static auto type_hash = ch(type_name);

		dScroller() :
			Component(type_name, type_hash)
		{
		}

		virtual void scroll(const vec2& v) = 0;

		FLAME_UNIVERSE_EXPORTS static dScroller* create();
	};
}
