#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cSlider : Component
	{
		inline static auto type_name = "flame::cSlider";
		inline static auto type_hash = ch(type_name);

		cSlider() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_value() const = 0;
		virtual void set_value(float v) = 0;
		virtual float get_value_min() const = 0;
		virtual void set_value_min(float v) = 0;
		virtual float get_value_max() const = 0;
		virtual void set_value_max(float v) = 0;

		FLAME_UNIVERSE_EXPORTS static cSlider* create();
	};
}
