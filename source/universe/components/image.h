#pragma once

#include "../component.h"

namespace flame
{
	struct cImage : Component
	{
		inline static auto type_name = "flame::cImage";
		inline static auto type_hash = ch(type_name);

		cImage() :
			Component(type_name, type_hash)
		{
		}

		virtual vec4 get_uv() const = 0;
		virtual void set_uv(const vec4& uv) = 0;

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* src) = 0;
		virtual const char* get_tile_name() const = 0;
		virtual void set_tile_name(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cImage* create(void* parms = nullptr);
	};
}
