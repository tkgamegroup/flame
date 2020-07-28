#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cImage : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "cImage";
		inline static auto type_hash = ch(type_name);

		cImage() :
			Component(type_name, type_hash, true)
		{
		}

		virtual uint get_res_id() const = 0;
		virtual void set_res_id(uint id) = 0;
		virtual uint get_tile_id() const = 0;
		virtual void set_tile_id(uint id) = 0;

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		FLAME_UNIVERSE_EXPORTS static cImage* create();
	};
}
