#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cSky : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cSky";
		inline static auto type_hash = ch(type_name);

		cSky() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_texture_name() const = 0;
		virtual void set_texture_name(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cSky* create();
	};
}
