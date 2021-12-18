#pragma once

#include "../component.h"

namespace flame
{
	struct cWindow : Component
	{
		inline static auto type_name = "flame::cWindow";
		inline static auto type_hash = ch(type_name);

		cWindow() : Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* title) = 0;

		virtual bool get_nomove() const = 0;
		virtual void set_nomove(bool v) = 0;
		virtual bool get_noresize() const = 0;
		virtual void set_noresize(bool v) = 0;

		virtual void* add_close_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_close_listener(void* lis) = 0;

		FLAME_UNIVERSE_EXPORTS static cWindow* create(void* parms = nullptr);
	};
}
