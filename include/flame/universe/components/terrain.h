#pragma once

#include <flame/universe/component.h>

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

		virtual uvec2 get_blocks() const = 0;
		virtual void set_blocks(const uvec2& b) = 0;
		virtual vec3 get_scale() const = 0;
		virtual void set_scale(const vec3& s) = 0;
		virtual uint get_tess_levels() const = 0;
		virtual void set_tess_levels(uint l) = 0;

		virtual const char* get_height_map() const = 0;
		virtual void set_height_map(const char* name) = 0;
		virtual const char* get_normal_map() const = 0;
		virtual void set_normal_map(const char* name) = 0;
		virtual const char* get_material_name() const = 0;
		virtual void set_material_name(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cTerrain* create();
	};
}
