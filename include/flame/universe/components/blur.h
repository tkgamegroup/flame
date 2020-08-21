#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cBlur : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cBlur";
		inline static auto type_hash = ch(type_name);

		cBlur() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_sigma() const = 0;
		virtual void set_sigma(float s) = 0;

		FLAME_UNIVERSE_EXPORTS static cBlur* create();
	};
}
