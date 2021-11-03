#pragma once

#include "../system.h"

#if USE_IMGUI
#include <imgui.h>
#endif

namespace flame
{
	struct sImgui : System
	{
		inline static auto type_name = "flame::sImgui";
		inline static auto type_hash = ch(type_name);

		sImgui() : System(type_name, type_hash)
		{
		}

		virtual void setup(graphics::Window* window) = 0;

		virtual void set_clear_color(const vec4& color) = 0;

		FLAME_UNIVERSE_EXPORTS static sImgui* create(void* parms = nullptr);
	};
}
