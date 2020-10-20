#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTerrain : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cTerrain";
		inline static auto type_hash = ch(type_name);

		cTerrain() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_height_map() const = 0;
		virtual void set_height_map(const char* name) = 0;
		virtual const char* get_color_map() const = 0;
		virtual void set_color_map(const char* name) = 0;

		virtual Vec2u get_blocks() const = 0;
		virtual void set_blocks(const Vec2u& b) = 0;
		virtual Vec3f get_scale() const = 0;
		virtual void set_scale(const Vec3f& s) = 0;
		virtual uint get_tess_levels() const = 0;
		virtual void set_tess_levels(uint l) = 0;

		FLAME_UNIVERSE_EXPORTS static cTerrain* create();
	};
}
