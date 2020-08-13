#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cImage : Component // R !ctor !dtor !type_name !type_hash
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

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		virtual bool get_auto_size() const = 0;
		virtual void set_auto_size(bool a) = 0;

		FLAME_UNIVERSE_EXPORTS static cImage* create();
	};
}
