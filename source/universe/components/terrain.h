#pragma once

#include "../component.h"

namespace flame
{
	struct cTerrain : Component
	{
		inline static auto type_name = "flame::cTerrain";
		inline static auto type_hash = ch(type_name);

		cTerrain() :
			Component(type_name, type_hash)
		{
		}

		virtual vec2 get_extent() const = 0;
		virtual void set_extent(const vec2& ext) = 0;
		virtual uint get_blocks() const = 0;
		virtual void set_blocks(uint b) = 0;
		virtual uint get_tess_levels() const = 0;
		virtual void set_tess_levels(uint l) = 0;

		virtual const char* get_height_map() const = 0;
		virtual void set_height_map(const char* name) = 0;
		virtual const char* get_material_name() const = 0;
		virtual void set_material_name(const char* name) = 0;

		virtual ShadingFlags get_shading_flags() const = 0;
		virtual void set_shading_flags(ShadingFlags flags) = 0;

		virtual graphics::Image* get_height_texture() const = 0;
		virtual graphics::Image* get_normal_texture() const = 0;
		virtual graphics::Image* get_tangent_texture() const = 0;

		FLAME_UNIVERSE_EXPORTS static cTerrain* create();
	};
}
