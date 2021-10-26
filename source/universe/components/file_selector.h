#pragma once

#include "../component.h"

namespace flame
{
	struct cFileSelector : Component
	{
		inline static auto type_name = "flame::cFileSelector";
		inline static auto type_hash = ch(type_name);

		cFileSelector() : Component(type_name, type_hash)
		{
		}

		virtual void set_folder(const wchar_t* path) = 0;

		virtual void* add_callback(void (*callback)(Capture& c, bool ok, const wchar_t* text), const Capture& capture) = 0;
		virtual void remove_callback(void* ret) = 0;

		FLAME_UNIVERSE_EXPORTS static cFileSelector* create(void* parms = nullptr);
	};
}
