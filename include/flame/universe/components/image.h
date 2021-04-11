#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cImage : Component
	{
		inline static auto type_name = "flame::cImage";
		inline static auto type_hash = ch(type_name);

		cImage() :
			Component(type_name, type_hash)
		{
		}

		virtual int get_res_id() const = 0;
		virtual void set_res_id(int id) = 0;
		virtual int get_tile_id() const = 0;
		virtual void set_tile_id(int id) = 0;

		virtual vec4 get_uv() const = 0;
		virtual void set_uv(const vec4& uv) = 0;

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		virtual void refresh_res() = 0;

		FLAME_UNIVERSE_EXPORTS static cImage* create(void* parms = nullptr);
	};
}
