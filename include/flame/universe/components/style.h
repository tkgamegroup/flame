#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cStyle : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cStyle";
		inline static auto type_hash = ch(type_name);

		cStyle() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_rule() const = 0;
		virtual void set_rule(const char* rule) = 0;

		FLAME_UNIVERSE_EXPORTS static cStyle* create();
	};
}
