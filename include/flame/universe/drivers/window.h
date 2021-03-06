#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dWindow : Driver
	{
		inline static auto type_name = "flame::dWindow";
		inline static auto type_hash = ch(type_name);

		dWindow() :
			Driver(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* title) = 0;

		virtual void* add_close_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_close_listener(void* lis) = 0;

		FLAME_UNIVERSE_EXPORTS static dWindow* create(void* parms = nullptr);
	};
}
