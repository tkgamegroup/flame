#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cObject : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cObject";
		inline static auto type_hash = ch(type_name);

		cObject() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		FLAME_UNIVERSE_EXPORTS static cObject* create();
	};
}
