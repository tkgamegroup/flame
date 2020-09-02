#pragma once

#include <flame/graphics/graphics.h>
#include <flame/universe/component.h>

namespace flame
{
	struct cLight : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cLight";
		inline static auto type_hash = ch(type_name);

		cLight() :
			Component(type_name, type_hash)
		{
		}

		virtual graphics::LightType get_type() const = 0;
		virtual void set_type(graphics::LightType t) = 0;

		FLAME_UNIVERSE_EXPORTS static cLight* create();
	};
}
