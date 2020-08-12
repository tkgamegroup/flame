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

		virtual ScrollerType get_type() const = 0;
		virtual void set_type(ScrollerType type) = 0;

		virtual void scroll(float v) = 0;

		FLAME_UNIVERSE_EXPORTS static cScroller* create();
	};

	struct cScrollView : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cScrollView";
		inline static auto type_hash = ch(type_name);

		cScrollView() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cScrollView* create();
	};
}
