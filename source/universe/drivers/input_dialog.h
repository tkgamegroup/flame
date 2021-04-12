#pragma once

#include "../driver.h"

namespace flame
{
	struct dInputDialog : Driver
	{
		inline static auto type_name = "flame::dInputDialog";
		inline static auto type_hash = ch(type_name);

		dInputDialog() :
			Driver(type_name, type_hash)
		{
		}

		virtual void set_text(const wchar_t* text) = 0;

		virtual void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) = 0;
		virtual void remove_callback(void* ret) = 0;

		FLAME_UNIVERSE_EXPORTS static dInputDialog* create(void* parms = nullptr);
	};
}
