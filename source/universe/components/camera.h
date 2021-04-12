#pragma once

#include "../component.h"

namespace flame
{
	struct cCamera : Component
	{
		inline static auto type_name = "flame::cCamera";
		inline static auto type_hash = ch(type_name);

		cCamera() :
			Component(type_name, type_hash)
		{
		}

		virtual bool get_current() const = 0;
		virtual void set_current(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static cCamera* create(void* parms = nullptr);
	};
}
