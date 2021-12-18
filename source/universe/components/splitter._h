#pragma once

#include "../component.h"

namespace flame
{
	struct cSplitter : Component
	{
		inline static auto type_name = "flame::cSplitter";
		inline static auto type_hash = ch(type_name);

		cSplitter() : Component(type_name, type_hash)
		{
		}

		virtual SplitterType get_type() const = 0;
		virtual void set_type(SplitterType v) = 0;

		FLAME_UNIVERSE_EXPORTS static cSplitter* create(void* parms = nullptr);
	};
}
