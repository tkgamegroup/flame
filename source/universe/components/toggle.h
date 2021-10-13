#pragma once

#include "../component.h"

namespace flame
{
	struct cToggle : Component
	{
		inline static auto type_name = "flame::cToggle";
		inline static auto type_hash = ch(type_name);
		
		cToggle() : Component(type_name, type_hash)
		{
		}
		
		virtual bool get_toggled() const = 0;
		virtual void set_toggled(bool v) = 0;
		
		FLAME_UNIVERSE_EXPORTS static cToggle* create(void* parms = nullptr);
	};
}
