#pragma once

#include "../component.h"

namespace flame
{
	struct cInputDialog : Component
	{
		inline static auto type_name = "flame::cInputDialog";
		inline static auto type_hash = ch(type_name);

		cInputDialog() : Component(type_name, type_hash)
		{
		}

		virtual void set_text(const wchar_t* v) = 0;

		virtual void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) = 0;
		virtual void remove_callback(void* ret) = 0;

		FLAME_UNIVERSE_EXPORTS static cInputDialog* create(void* parms = nullptr);
	};
}
