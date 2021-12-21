#pragma once

#include "../component.h"

namespace flame
{
	struct cWater : Component
	{
		inline static auto type_name = "flame::cWater";
		inline static auto type_hash = ch(type_name);

		cWater() :
			Component(type_name, type_hash)
		{
		}

		virtual vec2 get_extent() const = 0;
		virtual void set_extent(const vec2& ext) = 0;

		virtual const char* get_material_name() const = 0;
		virtual void set_material_name(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cWater* create();
	};
}
