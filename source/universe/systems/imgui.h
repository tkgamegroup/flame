#pragma once

#include "../system.h"

#include <imgui.h>

namespace flame
{
	struct sImgui : System
	{
		inline static auto type_name = "flame::sImgui";
		inline static auto type_hash = ch(type_name);

		sImgui() : System(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static sImgui* create(void* parms = nullptr);
	};
}
