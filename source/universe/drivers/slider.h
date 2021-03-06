#pragma once

#include "../driver.h"

namespace flame
{
	struct dSlider : Driver
	{
		inline static auto type_name = "flame::dSlider";
		inline static auto type_hash = ch(type_name);

		dSlider() :
			Driver(type_name, type_hash)
		{
		}

		virtual float get_value() const = 0;
		virtual void set_value(float v) = 0;
		virtual float get_value_min() const = 0;
		virtual void set_value_min(float v) = 0;
		virtual float get_value_max() const = 0;
		virtual void set_value_max(float v) = 0;

		FLAME_UNIVERSE_EXPORTS static dSlider* create(void* parms = nullptr);
	};
}
