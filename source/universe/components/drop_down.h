#pragma once

#include "../component.h"

namespace flame
{
	struct cDropDown : Component
	{
		inline static auto type_name = "flame::cDropDown";
		inline static auto type_hash = ch(type_name);

		cDropDown() : Component(type_name, type_hash)
		{
		}

		virtual int get_index() const = 0;
		virtual void set_index(int v) = 0;

		FLAME_UNIVERSE_EXPORTS static cDropDown* create(void* parms = nullptr);
	};
}
