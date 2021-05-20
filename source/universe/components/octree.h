#pragma once

#include "../component.h"

namespace flame
{
	struct cOctree : Component
	{
		inline static auto type_name = "flame::cOctree";
		inline static auto type_hash = ch(type_name);

		cOctree() :
			Component(type_name, type_hash)
		{
		}

		virtual float get_length() const = 0;
		virtual void set_length(float length) = 0;

		FLAME_UNIVERSE_EXPORTS static cOctree* create(void* parms = nullptr);
	};
}
