#pragma once

#include "../component.h"

namespace flame
{
	struct cMenuItem : Component
	{
		inline static auto type_name = "flame::cMenuItem";
		inline static auto type_hash = ch(type_name);
		
		cMenuItem() : Component(type_name, type_hash)
		{
		}
		
		virtual bool get_checkable() const = 0;
		virtual void set_checkable(bool v) = 0;
		
		virtual bool get_checked() const = 0;
		virtual void set_checked(bool v) = 0;
		
		virtual void set_radio_checked() = 0;
		
		FLAME_UNIVERSE_EXPORTS static cMenuItem* create(void* parms = nullptr);
	};
}
