#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cScript : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cScript";
		inline static auto type_hash = ch(type_name);

		cScript() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* fn) = 0;

		FLAME_UNIVERSE_EXPORTS static cScript* create();
	};
}
