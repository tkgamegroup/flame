#pragma once

#include "../component.h"

namespace flame
{
	struct cBlur : Component
	{
		inline static auto type_name = "flame::cBlur";
		inline static auto type_hash = ch(type_name);

		cBlur() :
			Component(type_name, type_hash)
		{
		}

		virtual uint get_blur_radius() const = 0;
		virtual void set_blur_radius(uint r) = 0;

		FLAME_UNIVERSE_EXPORTS static cBlur* create(void* parms = nullptr);
	};
}
