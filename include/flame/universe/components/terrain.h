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

		virtual Vec2u get_size() const = 0;
		virtual void set_size(const Vec2u& s) = 0;
		virtual Vec3f get_extent() const = 0;
		virtual void set_extent(const Vec3f& e) = 0;
		virtual float get_tess_levels() const = 0;
		virtual void set_tess_levels(float l) = 0;

		virtual const char* get_blend_map() const = 0;
		virtual void set_blend_map(const char* name) = 0;
		virtual const char* get_color_map_0() const = 0;
		virtual void set_color_map_0(const char* name) = 0;
		virtual const char* get_color_map_1() const = 0;
		virtual void set_color_map_1(const char* name) = 0;
		virtual const char* get_color_map_2() const = 0;
		virtual void set_color_map_2(const char* name) = 0;
		virtual const char* get_color_map_3() const = 0;
		virtual void set_color_map_3(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cTerrain* create();
	};
}
