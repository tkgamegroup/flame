#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cScroller : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cScroller";
		inline static auto type_hash = ch(type_name);

		cScroller() :
			Component(type_name, type_hash)
		{
		}

		virtual void scroll(const Vec2f& v) = 0;

		FLAME_UNIVERSE_EXPORTS static cScroller* create();
	};
}
