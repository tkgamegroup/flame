#pragma once

#include "../component.h"

namespace flame
{
	struct cImgui : Component
	{
		inline static auto type_name = "flame::cImgui";
		inline static auto type_hash = ch(type_name);

		cImgui() : Component(type_name, type_hash)
		{
		}

		virtual void on_draw(void (*draw)(Capture& c), const Capture& capture) = 0;

		FLAME_UNIVERSE_EXPORTS static cImgui* create(void* parms = nullptr);
	};
}
